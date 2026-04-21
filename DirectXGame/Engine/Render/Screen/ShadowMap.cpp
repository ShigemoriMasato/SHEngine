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

void SHEngine::Screen::ShadowMap::Clear(Command::Object* cmdObject) {
    cmdObject->GetCommandList()->ClearDepthStencilView(dsvHandle_->GetCPU(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
}

void SHEngine::Screen::ShadowMap::ToPresent(Command::Object* cmdObject) {
	TransitionBarrier(cmdObject, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

void SHEngine::Screen::ShadowMap::ToTexture(Command::Object* cmdObject) {
	ToPresent(cmdObject);
}

void SHEngine::Screen::ShadowMap::TransitionBarrier(CmdObj* cmdObj, D3D12_RESOURCE_STATES afterState) {
    Func::InsertBarrier(cmdObj->GetCommandList(), afterState, beforeState_, depthTextureData_->GetResource());
}
