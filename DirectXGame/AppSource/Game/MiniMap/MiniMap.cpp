#include "MiniMap.h"
#include <numbers>

void MiniMap::Initialize(int mapWidth, int mapHeight, TextureManager* textureManager) {
	// カメラの初期化
	camera_ = std::make_unique<Camera>();
	camera_->SetProjectionMatrix(PerspectiveFovDesc());

	transform_.rotate.x = std::numbers::pi_v<float> / 2.0f;
	distRatio_ = 0.63f;

	dist = std::max(mapWidth, mapHeight) * 0.5f / std::tan(0.45f / 2.0f);
	transform_.position = { mapWidth / 2.0f - 0.5f, 0.0f, mapHeight / 2.0f - 0.5f };
	Vector3 cameraPos = transform_.position + Vector3{ 0.0f, dist * distRatio_, 0.0f };
	camera_->SetTransform(Matrix::MakeAffineMatrix(transform_.scale, transform_.rotate, {cameraPos}));
	camera_->MakeMatrix();
	// デュアルディスプレイの初期化
	display_ = std::make_unique<DualDisplay>("MiniMapDisplay");
	int textureHandle1 = textureManager->CreateWindowTexture(1280, 720, 0xffffffff);
	int textureHandle2 = textureManager->CreateWindowTexture(1280, 720, 0xffffffff);
	display_->Initialize(textureManager->GetTextureData(textureHandle1), textureManager->GetTextureData(textureHandle2));
}

Camera* MiniMap::PreDraw(Window* window) {
	display_->PreDraw(window->GetCommandObject(), true);
	return camera_.get();
}

void MiniMap::PostDraw(Window* window) {
	display_->PostDraw(window->GetCommandObject());
}

void MiniMap::DrawImGui() {
#ifdef USE_IMGUI

	ImGui::Begin("MiniMap");

	TextureData* textureData = display_->GetTextureData();
	ImGui::Image((void*)(intptr_t)textureData->GetGPUHandle().ptr, ImVec2(400, 225));
	ImGui::DragFloat("Distance Ratio", &distRatio_, 0.01f);
	ImGui::DragFloat3("Scale", &transform_.scale.x, 0.1f);
	ImGui::DragFloat3("Rotate", &transform_.rotate.x, 0.1f);
	ImGui::DragFloat3("Position", &transform_.position.x, 0.1f);
	Vector3 cameraPos = transform_.position + Vector3{ 0.0f, dist * distRatio_, 0.0f };
	camera_->SetTransform(Matrix::MakeAffineMatrix(transform_.scale, transform_.rotate, { cameraPos }).Inverse());
	camera_->MakeMatrix();

	ImGui::End();

#endif
}
