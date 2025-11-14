#include "Display.h"
#include <Utility/InsertBarrier.h>
#include <Utility/Color.h>

ID3D12Device* Display::device_ = nullptr;
RTVManager* Display::rtvManager_ = nullptr;
DSVManager* Display::dsvManager_ = nullptr;

namespace {

    ID3D12Resource* CreateDepthStencilTextureResource(ID3D12Device* device, int32_t width, int32_t height) {
        //生成するResourceの設定
        D3D12_RESOURCE_DESC resourceDesc{};
        resourceDesc.Width = width;
        resourceDesc.Height = height;
        resourceDesc.MipLevels = 1;
        resourceDesc.DepthOrArraySize = 1;
        resourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        resourceDesc.SampleDesc.Count = 1;
        resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

        //利用するヒープの設定
        D3D12_HEAP_PROPERTIES heapProperties{};
        heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

        //深度値のクリア設定
        D3D12_CLEAR_VALUE depthClearValue{};
        depthClearValue.DepthStencil.Depth = 1.0f;
        depthClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

        //Resourceの生成
        ID3D12Resource* resource = nullptr;
        HRESULT hr = device->CreateCommittedResource(
            &heapProperties,
            D3D12_HEAP_FLAG_NONE,
            &resourceDesc,
            D3D12_RESOURCE_STATE_DEPTH_WRITE,
            &depthClearValue,
            IID_PPV_ARGS(&resource));
        assert(SUCCEEDED(hr));

        return resource;
    }

}

void Display::StaticInitialize(ID3D12Device* device, RTVManager* rtvManager, DSVManager* dsvManager) {
	device_ = device;
	rtvManager_ = rtvManager;
	dsvManager_ = dsvManager;
}

void Display::Initialize(TextureData* data, uint32_t clearColor) {
	textureResource_ = data->GetResource();
	width_ = data->GetSize().first;
	height_ = data->GetSize().second;
	clearColor_ = ConvertColor(clearColor);

    //バリアの初期状態を設定
    resourceState_ = D3D12_RESOURCE_STATE_RENDER_TARGET;

    //RTVの設定
    rtvHandle_.UpdateHandle(rtvManager_);

    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
    rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;	//出力結果をSRGBに変換して書き込む
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;	//2Dテクスチャとしてよみこむ
	device_->CreateRenderTargetView(textureResource_, &rtvDesc, rtvHandle_.GetCPU());

	//DSVの設定
    dsvHandle_.UpdateHandle(dsvManager_);

    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
    dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

    depthStencilResource_.Attach(CreateDepthStencilTextureResource(device_, width_, height_));

    device_->CreateDepthStencilView(depthStencilResource_.Get(), &dsvDesc, dsvHandle_.GetCPU());
}

void Display::DrawReady(ID3D12GraphicsCommandList* commandList, bool isClear) {
	EditBarrier(commandList, D3D12_RESOURCE_STATE_RENDER_TARGET);

    auto dsvCpu = dsvHandle_.GetCPU();
	auto rtvCpu = rtvHandle_.GetCPU();
	commandList->OMSetRenderTargets(1, &rtvCpu, false, &dsvCpu);

    if (isClear) {
        //レンダーターゲットのクリア
        commandList->ClearRenderTargetView(rtvHandle_.GetCPU(), reinterpret_cast<const FLOAT*>(&clearColor_), 0, nullptr);
        //デプスステンシルビューのクリア
        commandList->ClearDepthStencilView(dsvHandle_.GetCPU(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

        //ビューポート
        D3D12_VIEWPORT viewport{};
        //クライアント領域のサイズと一緒にして画面全体に表示
        viewport.Width = static_cast<float>(width_);
        viewport.Height = static_cast<float>(height_);
        viewport.TopLeftX = 0;
        viewport.TopLeftY = 0;
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;

        //シザー矩形
        D3D12_RECT scissorRect{};
        //基本的にビューポートと同じく刑が構成されるようにする
        scissorRect.left = 0;
        scissorRect.right = width_;
        scissorRect.top = 0;
        scissorRect.bottom = height_;

        commandList->RSSetViewports(1, &viewport);
        commandList->RSSetScissorRects(1, &scissorRect);
	}
}

void Display::EditBarrier(ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_STATES newState) {
	InsertBarrier(commandList, newState, resourceState_, textureResource_);
}
