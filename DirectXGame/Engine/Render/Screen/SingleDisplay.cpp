#include "SingleDisplay.h"
#include <Utility/DirectUtilFuncs.h>

namespace {

    ID3D12Resource* CreateDepthStencilTextureResource(ID3D12Device* device, int32_t width, int32_t height) {
        //生成するResourceの設定
        D3D12_RESOURCE_DESC resourceDesc{};
        resourceDesc.Width = width;
        resourceDesc.Height = height;
        resourceDesc.MipLevels = 1;
        resourceDesc.DepthOrArraySize = 1;
        resourceDesc.Format = DXGI_FORMAT_D32_FLOAT;
        resourceDesc.SampleDesc.Count = 1;
        resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

        //利用するヒープの設定
        D3D12_HEAP_PROPERTIES heapProperties{};
        heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

        //深度値のクリア設定
        D3D12_CLEAR_VALUE depthClearValue{};
        depthClearValue.DepthStencil.Depth = 1.0f;
        depthClearValue.Format = DXGI_FORMAT_D32_FLOAT;

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

void SHEngine::Screen::SingleDisplay::Initialize(TextureManager* textureManager, int width, int height, uint32_t clearColor) {
    ID3D12Device* device = device_->GetDevice();
    DSVManager* dsvManager = device_->GetDSVManager();
    RTVManager* rtvManager = device_->GetRTVManager();
    rtvFormat_ = DXGI_FORMAT_R8G8B8A8_UNORM;

    //テクスチャの生成
    int textureHandle = textureManager->CreateWindowTexture(width, height, clearColor);
    textureData_ = textureManager->GetTextureData(textureHandle);
    width_ = width;
    height_ = height;

    PrivateInitialize(textureManager);

	currentBarrier_ = D3D12_RESOURCE_STATE_PRESENT;

    isOffScreen_ = true;
}

void SHEngine::Screen::SingleDisplay::Initialize(TextureManager* textureManager, ID3D12Resource* resource, uint32_t clearColor) {
    rtvFormat_ = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

	//スワップチェーンのリソースからテクスチャを作成
	int textureHandle = textureManager->CreateSwapChainTexture(resource, clearColor);
	textureData_ = textureManager->GetTextureData(textureHandle);
	width_ = textureData_->GetSize().first;
	height_ = textureData_->GetSize().second;

    PrivateInitialize(textureManager);

	currentBarrier_ = D3D12_RESOURCE_STATE_PRESENT;

	isOffScreen_ = false;
}

void SHEngine::Screen::SingleDisplay::PrivateInitialize(SHEngine::TextureManager* textureManager) {
    ID3D12Device* device = device_->GetDevice();
    DSVManager* dsvManager = device_->GetDSVManager();
    RTVManager* rtvManager = device_->GetRTVManager();

    //RTVの設定
    rtvHandle_.UpdateHandle(rtvManager);
    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
    rtvDesc.Format = rtvFormat_;
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;	//2Dテクスチャとしてよみこむ
    device->CreateRenderTargetView(textureData_->GetResource(), &rtvDesc, rtvHandle_.GetCPU());

    //DSVの設定
    dsvHandle_.UpdateHandle(dsvManager);
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
    dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    auto row = CreateDepthStencilTextureResource(device, width_, height_);
    device->CreateDepthStencilView(row, &dsvDesc, dsvHandle_.GetCPU());
    int dsTextureIndex = textureManager->CreateDepthTexture(row);
    depthTextureData_ = textureManager->GetTextureData(dsTextureIndex);

}

void SHEngine::Screen::SingleDisplay::PreDraw(Command::Object* cmdObject, bool isClear) {
    TransitionBarrier(cmdObject, D3D12_RESOURCE_STATE_RENDER_TARGET);
	TransitionDepthBarrier(cmdObject, D3D12_RESOURCE_STATE_DEPTH_WRITE);
	auto commandList = cmdObject->GetCommandList();

    auto dsvCpu = dsvHandle_.GetCPU();
    auto rtvCpu = rtvHandle_.GetCPU();
    commandList->OMSetRenderTargets(1, &rtvCpu, false, &dsvCpu);


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

    if (isClear) {
		Vector4 clearColor = textureData_->GetClearColor();
        //レンダーターゲットのクリア
        commandList->ClearRenderTargetView(rtvHandle_.GetCPU(), &clearColor.x, 0, nullptr);
        //デプスステンシルビューのクリア
        commandList->ClearDepthStencilView(dsvHandle_.GetCPU(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
    }
}

void SHEngine::Screen::SingleDisplay::PostDraw(Command::Object* cmdObject) {
    if (isOffScreen_) {
		ToTexture(cmdObject);
    } else {
        TransitionBarrier(cmdObject, D3D12_RESOURCE_STATE_PRESENT);
        TransitionDepthBarrier(cmdObject, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    }
}

void SHEngine::Screen::SingleDisplay::ToTexture(Command::Object* cmdObject) {
	TransitionBarrier(cmdObject, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	TransitionDepthBarrier(cmdObject, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

void SHEngine::Screen::SingleDisplay::TransitionBarrier(Command::Object* cmdObject, D3D12_RESOURCE_STATES after) {
	SHEngine::Func::InsertBarrier(cmdObject->GetCommandList(), after, currentBarrier_, textureData_->GetResource());
}

void SHEngine::Screen::SingleDisplay::TransitionDepthBarrier(Command::Object* cmdObject, D3D12_RESOURCE_STATES after) {
	SHEngine::Func::InsertBarrier(cmdObject->GetCommandList(), after, currentDepthBarrier_, depthTextureData_->GetResource());
}
