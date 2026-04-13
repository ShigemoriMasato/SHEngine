#include "ShadowMap.h"
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

using namespace SHEngine::Screen;

void ShadowMap::Initialize(SHEngine::TextureManager* textureManager) {
	//DSVの作成
	auto dsv = device_->GetDSVManager();
	dsvHandle_ = std::make_unique<DSVHandle>();
	dsvHandle_->UpdateHandle(dsv);

	auto resource = CreateDepthStencilTextureResource(device_->GetDevice(), 2048, 2048);

	//テクスチャデータの作成
	int id = textureManager->CreateDepthTexture(resource);
	depthTextureData_ = textureManager->GetTextureData(id);
}

void SHEngine::Screen::ShadowMap::PreDraw(Command::Object* cmdObject, bool isClear) {
	auto cmdList = cmdObject->GetCommandList();
	//深度バッファを描画可能な状態にする
	TransitionBarrier(cmdObject, D3D12_RESOURCE_STATE_DEPTH_WRITE);
	//深度バッファをセット
	auto dsvHandle = dsvHandle_->GetCPU();
	cmdList->OMSetRenderTargets(0, nullptr, FALSE, &dsvHandle);

    //ビューポート
    D3D12_VIEWPORT viewport{};
    //クライアント領域のサイズと一緒にして画面全体に表示
    viewport.Width = static_cast<float>(depthTextureData_->GetSize().first);
    viewport.Height = static_cast<float>(depthTextureData_->GetSize().second);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    cmdList->RSSetViewports(1, &viewport);

    //シザー矩形
    D3D12_RECT scissorRect{};
    scissorRect.left = 0;
    scissorRect.top = 0;
    scissorRect.right = static_cast<LONG>(depthTextureData_->GetSize().first);
    scissorRect.bottom = static_cast<LONG>(depthTextureData_->GetSize().second);
    cmdList->RSSetScissorRects(1, &scissorRect);

    if (isClear) {
		cmdList->ClearDepthStencilView(dsvHandle_->GetCPU(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
    }
}

void SHEngine::Screen::ShadowMap::PostDraw(Command::Object* cmdObject) {
	TransitionBarrier(cmdObject, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

void SHEngine::Screen::ShadowMap::ToTexture(Command::Object* cmdObject) {
	PostDraw(cmdObject);
}

void SHEngine::Screen::ShadowMap::TransitionBarrier(CmdObj* cmdObj, D3D12_RESOURCE_STATES afterState) {
    Func::InsertBarrier(cmdObj->GetCommandList(), afterState, beforeState_, depthTextureData_->GetResource());
}
