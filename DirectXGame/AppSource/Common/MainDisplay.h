#pragma once
#include <Render/Screen/MultiDisplay.h>
#include <Input/Input.h>

class MainDisplay {
public:

	void Initialize(int width, int height, uint32_t clearColor, SHEngine::TextureManager* textureManager, SHEngine::Input* input);

	void PreDraw(SHEngine::Command::Object* cmdObject, bool isClear = true);
	void PostDraw(SHEngine::Command::Object* cmdObject);

	void DrawImGui();

	bool IsHovering() const { return isHovering_; }
	Vector2 GetCursorPos() const { return cursorPos_; }

	SHEngine::Screen::IDisplay* GetDisplay() { return mainDisplay_.get(); }
	SHEngine::TextureData* GetTextureData() const { return mainDisplay_->GetTextureData(); }

private:

	std::unique_ptr<SHEngine::Screen::MultiDisplay> mainDisplay_ = nullptr;
	SHEngine::Input* input_ = nullptr;

	Vector2 cursorPos_ = { 0.0f, 0.0f };
	bool isHovering_ = false;

};
