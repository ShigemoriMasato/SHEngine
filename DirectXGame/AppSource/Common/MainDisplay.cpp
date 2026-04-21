#include "MainDisplay.h"
#include <imgui/imgui.h>

void MainDisplay::Initialize(int width, int height, uint32_t clearColor, SHEngine::TextureManager* textureManager, SHEngine::Input* input) {
	mainDisplay_ = std::make_unique<SHEngine::Screen::MultiDisplay>();
	mainDisplay_->Initialize(width, height, clearColor, textureManager);
	input_ = input;
}

void MainDisplay::PreDraw(SHEngine::Command::Object* cmdObject, bool isClear) {
	cmdObject->SetRenderTarget(mainDisplay_.get(), isClear);
}

void MainDisplay::PostDraw(SHEngine::Command::Object* cmdObject) {
	mainDisplay_->ToPresent(cmdObject);
}

void MainDisplay::DrawImGui() {
#ifdef USE_IMGUI

	const Vector2 windowSize = { 1280, 720 };
	const Vector2 imageSize = { 640, 360 };

	ImGui::Begin("MainDisplay");
	ImVec2 windowPos = ImGui::GetCursorPos();
	ImGui::Image(ImTextureRef(mainDisplay_->GetTextureData()->GetGPUHandle().ptr), { imageSize.x, imageSize.y });

	isHovering_ = false;

	if (ImGui::IsItemHovered()) {
		isHovering_ = true;

		//マウスの左上をウィンドウに合わせる
		cursorPos_ = input_->GetCursorPos() - Vector2(windowPos.x, windowPos.y);

		//正規化
		cursorPos_.x *= windowSize.x / imageSize.x;
		cursorPos_.y *= windowSize.y / imageSize.y;
	}

	ImGui::End();

#endif
}
