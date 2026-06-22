#pragma once
#include <SHEngine.h>
#include <Camera/Camera.h>
#include "Data/DecoPathManager.h"
#include "Data/DecoObjectManager.h"
#include "Data/DecoObjectController.h"

class DecoEditor {
public:

	DecoEditor(SHEngine::Engine* engine, SHEngine::Screen::Display* mainDisplay);
	~DecoEditor();

	void Update(Camera* camera, DCC* dcc);
	void Draw(DCC* dcc);

	void Undo() { decoDataManager_->Undo(); }
	void Redo() { decoDataManager_->Redo(); }

private:

	SHEngine::Screen::Display* display_ = nullptr;

	std::unique_ptr<Decorate::PathManager> decoPathManager_;
	std::unique_ptr<Decorate::DataManager> decoDataManager_;
	std::unique_ptr<Decorate::ObjManager> decoObjectManager_;

	std::unique_ptr<Decorate::ObjController> decoObjectController_;
};
