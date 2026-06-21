#pragma once
#include <Scene/IScene.h>
#include <Tool/DecoEditor/DecoEditor.h>
#include <Camera/DebugCamera.h>
#include <Tool/Grid/Grid.h>

class EditScene : public IScene {
public:

	EditScene() {};
	~EditScene() override = default;

	void Initialize() override;
	
	std::unique_ptr<IScene> Update() override;
	
	void Draw() override;

private:

	DebugCamera debugCamera_;
	Grid grid_;

	std::unique_ptr<DecoEditor> decoEditor_ = nullptr;	

};

