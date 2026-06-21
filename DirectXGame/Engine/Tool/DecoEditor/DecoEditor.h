#pragma once
#include <SHEngine.h>
#include <Camera/Camera.h>
#include "Data/DecoPathManager.h"
#include "Data/DecoObjectManager.h"
#include "Data/DecoObjectController.h"

class DecoEditor {
public:

	DecoEditor(SHEngine::Engine* engine, SHEngine::Screen::Display* mainDisplay);

	void Update(Camera* camera, DCC* dcc);

	void Draw(DCC* dcc);

private:

	SHEngine::Screen::Display* display_ = nullptr;

	std::unique_ptr<Decorate::DecoPathManager> decoPathManager_;
	std::unique_ptr<Decorate::ObjManager> decoObjectManager_;

	std::unique_ptr<Decorate::ObjController> decoObjectController_;
};
