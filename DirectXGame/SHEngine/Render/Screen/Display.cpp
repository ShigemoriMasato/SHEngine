#include "Display.h"
#include <Utility/DirectUtilFuncs.h>
#include <Utility/Color.h>
#include <imgui/imgui.h>
#include <imgui/ImGuizmo.h>

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
	if (depthTexture_ != nullptr) {
		assert(false && "既に深度テクスチャが作成されています。");
		return;
	}

	int dsTextureIndex = textureManager->CreateDepthTexture(uint32_t(width_), uint32_t(height_));
	depthTexture_ = textureManager->GetDepthTexture(dsTextureIndex);
	dsvHandlePtr_ = depthTexture_->GetDSVHandle();

	logger_->debug("Depth texture created. Size: {}x{}", width_, height_);
}

void SHEngine::Screen::Display::AddRenderTarget(TextureManager* textureManager, ID3D12Resource* resource, uint32_t clearColor) {
	if (!renderTextures_.empty()) {
		logger_->error("Swapchain用のDisplayはAddRenderTargetを一度しか呼べません。");
		return;
	}
	isOffScreen_ = false;

	int textureHandle = textureManager->CreateSwapChainTexture(resource, clearColor);
	AddRenderTexture(textureManager->GetRenderTexture(textureHandle));

	auto size = renderTextures_.back()->GetSize();
	if (width_ != int(size.first) || height_ != int(size.second)) {
		logger_->error("SwapchainのサイズとDisplayのサイズが一致しません。早急に初期化時の値とサイズを合わせてください。");
		width_ = size.first;
		height_ = size.second;
		if (depthTexture_ != nullptr) {
			logger_->error("Size変更に伴い、DepthTextureを再作成します。");
			depthTexture_ = nullptr;
			CreateDepthTexture(textureManager);
		}
	}
}

void SHEngine::Screen::Display::AddRenderTarget(TextureManager* textureManager, uint32_t clearColor, Format format, bool unordered) {
	if (!isOffScreen_) {
		//初期値がtrueで、falseにできるのがswapchain用のAddRenderTargetだけなので、ここでfalseであれば、既にSwapchain用のDisplayが作成されていることになる。
		assert(isOffScreen_ && "既にSwapchain用のDisplayが作成されています。");
		return;
	}

	unordered_ = unordered;
	isOffScreen_ = true;

	int textureHandle = textureManager->CreateWindowTexture(width_, height_, clearColor, format, unordered);
	AddRenderTexture(textureManager->GetRenderTexture(textureHandle));
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

	for (int i = 0; i < renderTextures_.size(); ++i) {
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

		ImGui::Image(ImTextureRef(renderTextures_[i]->GetSRVHandle().ptr), imageSize);

		if (i == 0) {
			ImVec2 windowPos = ImGui::GetItemRectMin();
			imguiWidth_ = imageSize.x;
			imguiHeight_ = imageSize.y;
			this->imguiPos_ = { windowPos.x, windowPos.y };
			isForcus_ = ImGui::IsWindowFocused();

			auto drawList = ImGui::GetWindowDrawList();
			ImGuizmo::SetDrawlist(drawList);
		}

		ImGui::End();
	}

#endif // USE_IMGUI

}

const ImGuiPayload* SHEngine::Screen::Display::DrawImGuiWithDD(std::string key) {
	const ImGuiPayload* payload = nullptr;

#ifdef USE_IMGUI

	Vector2 aspectRatio = { static_cast<float>(width_) / height_, 1.0f };

	for (int i = 0; i < renderTextures_.size(); ++i) {
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

		ImGui::Image(ImTextureRef(renderTextures_[i]->GetSRVHandle().ptr), imageSize);

		if (i == 0) {

			isForcus_ = ImGui::IsWindowFocused();

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

	// マルチビューポートでは imguiPos_ がデスクトップ座標になるため、
	// windowCursor と同じメインウィンドウのクライアント座標に揃える
	const ImVec2 mainPos = ImGui::GetMainViewport()->Pos;
	windowPos.x -= mainPos.x;
	windowPos.y -= mainPos.y;

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

bool SHEngine::Screen::Display::IsForcus() {
#ifdef USE_IMGUI
	return isForcus_;
#endif

	return true;
}

void SHEngine::Screen::Display::AddRenderTexture(RenderTexture* renderTexture) {
	assert(renderTexture && "Display::AddRenderTexture: renderTexture is nullptr");
	renderTextures_.push_back(renderTexture);
	rtvHandlePtr_.push_back(renderTexture->GetRTVHandle());

	//いつか配列にする
	rtvFormat_ = renderTexture->GetFormat();
}

void SHEngine::Screen::Display::Clear(DCC* dcc) {
	auto cmdList = dcc->GetCommandList();
	//RenderTargetをクリア
	for (size_t i = 0; i < renderTextures_.size(); ++i) {
		Vector4 color = renderTextures_[i]->GetClearColor();
		cmdList->ClearRenderTargetView(GetRTVHandle()[i], &color.x, 0, nullptr);
	}

	//DepthStencilをクリア
	if (depthTexture_) {
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
	TransitionBarrier(dcc, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
	TransitionDepthBarrier(dcc, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
}

void SHEngine::Screen::Display::ToNonPixel(DCC* dcc) {
	TransitionBarrier(dcc, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	TransitionDepthBarrier(dcc, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
}

void SHEngine::Screen::Display::ToUnordered(SHEngine::ICommandContext* dcc, bool depthToo) {
	if (!unordered_) {
		logger_->warn("Display {} is not created as unordered access. ToUnordered() is ignore.", windowName_);
	}

	TransitionBarrier(dcc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	if (depthToo) {
		TransitionDepthBarrier(dcc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	} else {
		TransitionDepthBarrier(dcc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	}
}

D3D12_CPU_DESCRIPTOR_HANDLE* SHEngine::Screen::Display::GetRTVHandle() {
	return rtvHandlePtr_.data();
}

D3D12_CPU_DESCRIPTOR_HANDLE* SHEngine::Screen::Display::GetDSVHandle() {
	return &dsvHandlePtr_;
}

void SHEngine::Screen::Display::TransitionBarrier(SHEngine::ICommandContext* dcc, D3D12_RESOURCE_STATES after) {
	auto cmdList = dcc->GetCommandList();
	for (auto* renderTexture : renderTextures_) {
		renderTexture->TransitionBarrier(after);
		renderTexture->Flush(cmdList);
	}
}

void SHEngine::Screen::Display::TransitionDepthBarrier(SHEngine::ICommandContext* dcc, D3D12_RESOURCE_STATES after) {
	auto cmdList = dcc->GetCommandList();
	if (depthTexture_) {
		depthTexture_->TransitionBarrier(after);
		depthTexture_->Flush(cmdList);
	}
}
