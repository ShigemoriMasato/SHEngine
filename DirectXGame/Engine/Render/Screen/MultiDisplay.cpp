#include "MultiDisplay.h"
#include "SingleDisplay.h"

using namespace SHEngine;

void SHEngine::Screen::MultiDisplay::Initialize(int width, int height, uint32_t clearColor, TextureManager* textureManager) {
	for (int i = 0; i < 1; ++i) {
		auto display = std::make_unique<SingleDisplay>();
		display->Initialize(textureManager, width, height, clearColor);
		displays_.push_back(std::move(display));
	}

	currentDisplayIndex_ = 0;
}

void SHEngine::Screen::MultiDisplay::PreDraw(Command::Object* cmdObject, bool isClear) {
	displays_[currentDisplayIndex_]->PreDraw(cmdObject, isClear);
}

void SHEngine::Screen::MultiDisplay::PostDraw(Command::Object* cmdObject) {
	displays_[currentDisplayIndex_]->PostDraw(cmdObject);
	currentDisplayIndex_ = (currentDisplayIndex_ + 1) % displays_.size();
}

void SHEngine::Screen::MultiDisplay::ToTexture(Command::Object* cmdObject) {
	displays_[currentDisplayIndex_]->ToTexture(cmdObject);
}
