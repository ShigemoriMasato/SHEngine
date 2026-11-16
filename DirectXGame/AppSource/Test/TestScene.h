#pragma once
#include <Scene/IScene.h>
#include <Camera/DebugCamera.h>
#include <Tool/Grid/Grid.h>

#include <Tool/DecoEditor/DecoEditor.h>

#include <Tool/ModelDrawer/ModelDrawer.h>

#include <Tool/CurveEditor/CurvePlayer.h>
#include <Tool/CurveEditor/CurveEditor.h>

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


	ModelDrawer cameraRenderer_{};

	CurveEditor curveEditor_{ "TestCurve" };
	std::vector<CurveBinder> curveBinders_;

	Vector3 testParam_;
	float testParam2_ = 0.0f;

private:

	char currentFileName_[256] = "";
	const std::string basePath_ = "Game/StageConfig/";
	const std::string extension_ = ".bin";

	std::vector<std::string> fileList_ = {};

	
};
