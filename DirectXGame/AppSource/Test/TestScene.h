#pragma once
#include <Scene/IScene.h>
#include <Camera/DebugCamera.h>
#include <Tool/Grid/Grid.h>

#include <Camera/Editor/CameraEditor.h>
#include <Tool/DecoEditor/DecoEditor.h>

#include <Tool/ModelDrawer/ModelDrawer.h>

#include "StageEditor.h"

class TestScene : public IScene {
public:

	~TestScene();

	void Initialize() override;
	std::unique_ptr<IScene> Update() override;
	void Draw() override;

private:

	void Save();
	void Load();

	void SelectFile();

	std::unique_ptr<DebugCamera> debugCamera_;
	std::unique_ptr<Camera> gameCamera_;
	Camera orthoCamera_;
	std::unique_ptr<Grid> grid_;

	CameraEditor cameraEditor_;
	std::unique_ptr<DecoEditor> decoEditor_;

	CameraCurveData cameraCurveData_;
	DecoObjData decoObjData_;

	std::vector<std::unique_ptr<ModelDrawer>> models_{};

private:

	char currentFileName_[256] = "";
	const std::string basePath_ = "Preset/";
	const std::string extension_ = ".bin";

	std::vector<std::string> fileList_ = {};

	StageEditor stageEditor_;
};
