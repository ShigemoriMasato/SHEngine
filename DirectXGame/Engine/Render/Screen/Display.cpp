#include "Display.h"
#include <Utility/DirectUtilFuncs.h>
#include <Utility/Color.h>
#include <imgui/imgui.h>
#include <imgui/ImGuizmo.h>

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

void SHEngine::Screen::Display::Initialize(int width, int height, std::string windowName) {
	width_ = width;
	height_ = height;

	if (windowName != "") {
		windowName_ = windowName;
	} else {
		static int displayCount = 0;
		windowName_ = "NoName_" + std::format("{:02d}", displayCount++);
	}

	logger_->info("Display {} initialized. Size: {}x{}", windowName_, width_, height_);
}

void SHEngine::Screen::Display::CreateDepthTexture(TextureManager* textureManager) {
	auto device = device_->GetDevice();
	auto dsvManager = device_->GetDSVManager();

	if (depthTextureData_ != nullptr) {
		assert(false && "既に深度テクスチャが作成されています。");
		return;
	}

	//DSVの設定(共有で一つ)
	dsvHandle_.UpdateHandle(dsvManager);
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	auto row = CreateDepthStencilTextureResource(device, width_, height_);
	device->CreateDepthStencilView(row, &dsvDesc, dsvHandle_.GetCPU());
	int dsTextureIndex = textureManager->CreateDepthTexture(row);
	depthTextureData_ = textureManager->GetTextureData(dsTextureIndex);
	dsvHandlePtr_ = dsvHandle_.GetCPU();

	logger_->debug("Depth texture created. Size: {}x{}");
}

void SHEngine::Screen::Display::AddRenderTarget(TextureManager* textureManager, ID3D12Resource* resource, uint32_t clearColor) {
	if (!textureData_.empty()) {
		logger_->error("Swapchain用のDisplayはAddRenderTargetを一度しか呼べません。");
		return;
	}
	isOffScreen_ = false;
	rtvFormat_ = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	
	int textureHandle = textureManager->CreateSwapChainTexture(resource, clearColor);
	textureData_.push_back(textureManager->GetTextureData(textureHandle));

	auto size = textureData_.back()->GetSize();
	if (width_ != size.first || height_ != size.second) {
		logger_->error("SwapchainのサイズとDisplayのサイズが一致しません。早急に初期化時の値とサイズを合わせてください。");
		width_ = size.first;
		height_ = size.second;
		if (depthTextureData_ != nullptr) {
			logger_->error("Size変更に伴い、DepthTextureを再作成します。");
			depthTextureData_ = nullptr;
			CreateDepthTexture(textureManager);
		}
	}

	CreateRenderTarget(textureManager, 0);
}

void SHEngine::Screen::Display::AddRenderTarget(TextureManager* textureManager, uint32_t clearColor, Format format) {
	if (!isOffScreen_) {
		//初期値がtrueで、falseにできるのがswapchain用のAddRenderTargetだけなので、ここでfalseであれば、既にSwapchain用のDisplayが作成されていることになる。
		assert(isOffScreen_ && "既にSwapchain用のDisplayが作成されています。");
		return;
	}

	isOffScreen_ = true;
	rtvFormat_ = DXGI_FORMAT_R8G8B8A8_UNORM;

	int textureHandle = textureManager->CreateWindowTexture(width_, height_, clearColor, format);
	textureData_.push_back(textureManager->GetTextureData(textureHandle));

	CreateRenderTarget(textureManager, uint32_t(textureData_.size() - 1));
}

void SHEngine::Screen::Display::SetViewport(DCC* dcc, Vector2 min, Vector2 size) {
	if (size.x == 0 && size.y == 0) {
		size = { static_cast<float>(width_), static_cast<float>(height_) };
	}

	SetViewportInPrivate(dcc, min, size);
}

void SHEngine::Screen::Display::DrawImGui() {
#ifdef USE_IMGUI

	Vector2 aspectRatio = { static_cast<float>(width_) / height_, 1.0f };

	for (int i = 0; i < textureData_.size(); ++i) {
		std::string nameStr = windowName_ + (i != 0 ? std::string("_") + std::to_string(i) : "");
		ImGui::Begin(nameStr.c_str());

		ImVec2 windowSize = ImGui::GetContentRegionAvail();
		float ratio;
		if (windowSize.x / windowSize.y > aspectRatio.x) {
			ratio = windowSize.y / height_;
		} else {
			ratio = windowSize.x / width_;
		}
		ImVec2 imageSize = ImVec2(width_ * ratio, height_ * ratio);

		ImGui::Image(ImTextureRef(textureData_[i]->GetGPUHandle().ptr), imageSize);
		ImGui::End();
	}

#endif // USE_IMGUI

}

const ImGuiPayload* SHEngine::Screen::Display::DrawImGuiWithDD(std::string key) {
	const ImGuiPayload* payload = nullptr;

#ifdef USE_IMGUI

	Vector2 aspectRatio = { static_cast<float>(width_) / height_, 1.0f };

	for (int i = 0; i < textureData_.size(); ++i) {
		std::string nameStr = windowName_ + (i != 0 ? std::string("_") + std::to_string(i) : "");
		ImGui::Begin(nameStr.c_str());

		ImVec2 windowSize = ImGui::GetContentRegionAvail();
		float ratio;
		if (windowSize.x / windowSize.y > aspectRatio.x) {
			ratio = windowSize.y / height_;
		} else {
			ratio = windowSize.x / width_;
		}
		ImVec2 imageSize = ImVec2(width_ * ratio, height_ * ratio);

		ImGui::Image(ImTextureRef(textureData_[i]->GetGPUHandle().ptr), imageSize);

		if (i == 0) {

			imguiWidth_ = imageSize.x;
			imguiHeight_ = imageSize.y;

			ImVec2 min = ImGui::GetItemRectMin();
			imguiPos_ = { min.x, min.y };

			if (ImGui::BeginDragDropTarget()) {
				auto tmp = ImGui::AcceptDragDropPayload(key.c_str());
				if (tmp) {
					payload = tmp;
				}
			}

			auto drawList = ImGui::GetWindowDrawList();
			ImGuizmo::SetDrawlist(drawList);
		}

		ImGui::End();
	}

#endif // USE_IMGUI

	return payload;
}

Vector2 SHEngine::Screen::Display::GetCursorPos(Vector2 windowCursor) {
#ifdef USE_IMGUI

	ImGui::Begin(windowName_.c_str());
	ImVec2 windowPos = { imguiPos_.x, imguiPos_.y };
	ImVec2 windowSize = { imguiWidth_, imguiHeight_ };
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

void SHEngine::Screen::Display::CreateRenderTarget(SHEngine::TextureManager* textureManager, uint32_t index) {
	ID3D12Device* device = device_->GetDevice();
	DSVManager* dsvManager = device_->GetDSVManager();
	RTVManager* rtvManager = device_->GetRTVManager();

	auto data = textureData_[index];
	rtvHandle_.resize(textureData_.size());
	rtvHandlePtr_.resize(textureData_.size());

	//いつか配列にする
	rtvFormat_ = data->GetFormat();

	rtvHandle_[index] = std::make_unique<RTVHandle>();

	//RTVの設定(指定された個数)
	rtvHandle_[index]->UpdateHandle(rtvManager);
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = data->GetFormat();
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;	//2Dテクスチャとしてよみこむ
	rtvHandlePtr_[index] = rtvHandle_[index]->GetCPU();
	device->CreateRenderTargetView(textureData_[index]->GetResource(), &rtvDesc, rtvHandlePtr_[index]);

	if (currentBarrier_.size() < textureData_.size()) {
		currentBarrier_.resize(textureData_.size(), D3D12_RESOURCE_STATE_COMMON);
	}
}

void SHEngine::Screen::Display::Clear(DCC* dcc) {
	auto cmdList = dcc->GetCommandList();
	//RenderTargetをクリア
	for (size_t i = 0; i < textureData_.size(); ++i) {
		Vector4 color = textureData_[i]->GetClearColor();
		cmdList->ClearRenderTargetView(GetRTVHandle()[i], &color.x, 0, nullptr);
	}

	//DepthStencilをクリア
	if (depthTextureData_) {
		cmdList->ClearDepthStencilView(*GetDSVHandle(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	}
}

void SHEngine::Screen::Display::ToRenderTarget(DCC* dcc) {
	TransitionBarrier(dcc, D3D12_RESOURCE_STATE_RENDER_TARGET);
	TransitionDepthBarrier(dcc, D3D12_RESOURCE_STATE_DEPTH_WRITE);
}

void SHEngine::Screen::Display::ToPresent(DCC* dcc) {
	if (isOffScreen_) {
		ToTexture(dcc);
	} else {
		TransitionBarrier(dcc, D3D12_RESOURCE_STATE_PRESENT);
		TransitionDepthBarrier(dcc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	}
}

void SHEngine::Screen::Display::ToTexture(DCC* dcc) {
	TransitionBarrier(dcc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	TransitionDepthBarrier(dcc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

void SHEngine::Screen::Display::ToNonPixel(DCC* dcc) {
	TransitionBarrier(dcc, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	TransitionDepthBarrier(dcc, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
}

D3D12_CPU_DESCRIPTOR_HANDLE* SHEngine::Screen::Display::GetRTVHandle() {
	return rtvHandlePtr_.data();
}

D3D12_CPU_DESCRIPTOR_HANDLE* SHEngine::Screen::Display::GetDSVHandle() {
	return &dsvHandlePtr_;
}

void SHEngine::Screen::Display::TransitionBarrier(DCC* dcc, D3D12_RESOURCE_STATES after) {
	auto cmdList = dcc->GetCommandList();
	for (size_t i = 0; i < textureData_.size(); ++i) {
		SHEngine::Func::InsertBarrier(cmdList, after, currentBarrier_[i], textureData_[i]->GetResource());
	}
}

void SHEngine::Screen::Display::TransitionDepthBarrier(DCC* dcc, D3D12_RESOURCE_STATES after) {
	auto cmdList = dcc->GetCommandList();
	if (depthTextureData_) {
		SHEngine::Func::InsertBarrier(cmdList, after, currentDepthBarrier_, depthTextureData_->GetResource());
	}
}
