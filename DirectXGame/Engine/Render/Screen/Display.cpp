#include "Display.h"
#include <Utility/DirectUtilFuncs.h>
#include <imgui/imgui.h>

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

void SHEngine::Screen::Display::Initialize(TextureManager* textureManager, int width, int height, uint32_t clearColor, std::string windowName) {
    ID3D12Device* device = device_->GetDevice();
    DSVManager* dsvManager = device_->GetDSVManager();
    RTVManager* rtvManager = device_->GetRTVManager();
    rtvFormat_ = DXGI_FORMAT_R8G8B8A8_UNORM;

    //テクスチャの生成
    int textureHandle = textureManager->CreateWindowTexture(width, height, clearColor);
    textureData_ = textureManager->GetTextureData(textureHandle);
    width_ = width;
    height_ = height;

    PrivateInitialize(textureManager, windowName);

	currentBarrier_ = D3D12_RESOURCE_STATE_PRESENT;

    isOffScreen_ = true;
}

void SHEngine::Screen::Display::Initialize(TextureManager* textureManager, ID3D12Resource* resource, uint32_t clearColor, std::string windowName) {
    rtvFormat_ = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

	//スワップチェーンのリソースからテクスチャを作成
	int textureHandle = textureManager->CreateSwapChainTexture(resource, clearColor);
	textureData_ = textureManager->GetTextureData(textureHandle);
	width_ = textureData_->GetSize().first;
	height_ = textureData_->GetSize().second;

    PrivateInitialize(textureManager, windowName);

	currentBarrier_ = D3D12_RESOURCE_STATE_PRESENT;

	isOffScreen_ = false;
}

void SHEngine::Screen::Display::DrawImGui() {
#ifdef USE_IMGUI

	Vector2 aspectRatio = { static_cast<float>(width_) / height_, 1.0f };

    ImGui::Begin(windowName_.c_str());

	ImVec2 windowSize = ImGui::GetContentRegionAvail();
    float ratio;
    if (windowSize.x / windowSize.y > aspectRatio.x) {
		ratio = windowSize.y / height_;
    } else {
		ratio = windowSize.x / width_;
    }
	ImVec2 imageSize = ImVec2(width_ * ratio, height_ * ratio);

	ImGui::Image(ImTextureRef(textureData_->GetGPUHandle().ptr), imageSize);
    ImGui::End();

#endif // USE_IMGUI

}

Vector2 SHEngine::Screen::Display::GetCursorPos(Vector2 windowCursor) {
#ifdef USE_IMGUI

	ImGui::Begin(windowName_.c_str());
    ImVec2 windowPos = ImGui::GetCursorPos();
	ImVec2 windowSize = ImGui::GetContentRegionAvail();
	ImGui::End();
	//ウィンドウ基準のカーソル位置をImGui上の位置に変換
	Vector2 imguiCursor;
	imguiCursor.x = (windowCursor.x - windowPos.x) / windowSize.x * width_;
	imguiCursor.y = (windowCursor.y - windowPos.y) / windowSize.y * height_;

    imguiCursor = {
        std::clamp(imguiCursor.x, 0.0f, static_cast<float>(width_)),
        std::clamp(imguiCursor.y, 0.0f, static_cast<float>(height_))
    };

	return imguiCursor;

#endif

    return windowCursor;
}

bool SHEngine::Screen::Display::IsHovering() {
	bool isHovering = true;

#ifdef USE_IMGUI

	ImGui::Begin(windowName_.c_str());
	isHovering = ImGui::IsWindowHovered();
	ImGui::End();

#endif // USE_IMGUI

	return isHovering;
}

void SHEngine::Screen::Display::PrivateInitialize(SHEngine::TextureManager* textureManager, std::string windowName) {
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

	if (windowName != "") {
		windowName_ = windowName;
	} else {
		static int displayCount = 0;
		windowName_ = "NoName_" + std::format("{:02d}", displayCount++);
	}
}

void SHEngine::Screen::Display::Clear(Command::Object* cmdObject) {
    Vector4 color = textureData_->GetClearColor();
    //レンダーターゲットと深度ステンシルをクリア
    cmdObject->GetCommandList()->ClearRenderTargetView(GetRTVHandle(), &color.x, 0, nullptr);
	cmdObject->GetCommandList()->ClearDepthStencilView(GetDSVHandle(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
}

void SHEngine::Screen::Display::ToRenderTarget(Command::Object* cmdObject) {
    TransitionBarrier(cmdObject, D3D12_RESOURCE_STATE_RENDER_TARGET);
	TransitionDepthBarrier(cmdObject, D3D12_RESOURCE_STATE_DEPTH_WRITE);
}

void SHEngine::Screen::Display::ToPresent(Command::Object* cmdObject) {
    if (isOffScreen_) {
		ToTexture(cmdObject);
    } else {
        TransitionBarrier(cmdObject, D3D12_RESOURCE_STATE_PRESENT);
        TransitionDepthBarrier(cmdObject, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    }
}

void SHEngine::Screen::Display::ToTexture(Command::Object* cmdObject) {
	TransitionBarrier(cmdObject, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	TransitionDepthBarrier(cmdObject, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

void SHEngine::Screen::Display::TransitionBarrier(Command::Object* cmdObject, D3D12_RESOURCE_STATES after) {
	SHEngine::Func::InsertBarrier(cmdObject->GetCommandList(), after, currentBarrier_, textureData_->GetResource());
}

void SHEngine::Screen::Display::TransitionDepthBarrier(Command::Object* cmdObject, D3D12_RESOURCE_STATES after) {
	SHEngine::Func::InsertBarrier(cmdObject->GetCommandList(), after, currentDepthBarrier_, depthTextureData_->GetResource());
}
