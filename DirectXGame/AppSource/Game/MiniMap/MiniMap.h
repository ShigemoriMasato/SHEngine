#pragma once
#include <Camera/Camera.h>
#include <Screen/DualDisplay.h>
#include <Screen/Window.h>
#include <Utility/DataStructures.h>

class MiniMap {
public:

	void Initialize(int mapWidth, int mapHeight, TextureManager* textureManager);
	Camera* PreDraw(Window* window);
	void PostDraw(Window* window);
	void DrawImGui();

	int GetTextureIndex() const { display_->GetTextureData()->GetOffset(); }

private:

	std::unique_ptr<Camera> camera_ = nullptr;
	std::unique_ptr<DualDisplay> display_ = nullptr;
	Transform transform_{};

	float distRatio_ = 0.2f;
	float dist = 0.0f;

};
