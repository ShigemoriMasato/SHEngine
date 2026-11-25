#pragma once
#include <Scene/IScene.h>
#include <Camera/DebugCamera.h>
#include <Tool/Grid/Grid.h>
#include <Particle/ParticleSystem.h>
#include <Particle/EmitFunc/InnerModel.h>

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

	std::unique_ptr<DebugCamera> debugCamera_;
	std::unique_ptr<Camera> gameCamera_;
	Camera orthoCamera_;
	std::unique_ptr<Grid> grid_;

	ParticleSystem particleSystem_{ 8388608 };
	Particle::Emitter* emitter_ = nullptr;

	std::unique_ptr<SHEngine::BufferContainer> container_;

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
