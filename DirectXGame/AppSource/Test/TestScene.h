#pragma once
#include <Scene/IScene.h>
#include <Camera/DebugCamera.h>
#include <Tool/Grid/Grid.h>
#include <Particle/ParticleSystem.h>
#include <Particle/EmitFunc/InnerModel.h>

#include <Tool/CurveEditor/CurvePlayer.h>
#include <Tool/CurveEditor/CurveEditor.h>

#include <Compute/Graph/ComputeTrigger.h>
#include <Render/Buffer/ReadBackBuffer.h>

class TestScene : public IScene {
public:

	~TestScene();

	void Initialize() override;
	std::unique_ptr<IScene> Update() override;
	void Draw() override;

private:

	struct Sphere {
		Vector3 center;
		float radius;
	};

	struct Config {
		int sphereCount = 1;
	};

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

	// ===================== ComputeGraphのテスト =================================

	void InitializeGraphTest();
	void UpdateGraphTest();
	// @brief Fill → Add の見本グラフを作る(今あるノードは消える)
	void BuildSampleGraph();

	struct FillConfig {
		float scale = 2.0f;
		float offset = 1.0f;
		uint32_t count = 64;
		float padding = 0.0f;
	};
	struct AddConfig {
		float add = 100.0f;
		float padding[3] = {};
	};
	static constexpr uint32_t kGraphTestCount = 64;

	std::unique_ptr<SHEngine::BufferContainer> graphTestContainer_;
	SHEngine::GPUBuffer* fillConfigBuffer_ = nullptr;	// CBV
	SHEngine::GPUBuffer* addConfigBuffer_ = nullptr;	// CBV
	SHEngine::GPUBuffer* inputBuffer_ = nullptr;		// SRV float x64
	SHEngine::GPUBuffer* outputBuffer_ = nullptr;		// SRV|UAV float x64 (引数 "Output" としてBind)
	SHEngine::GPUBuffer* counterBuffer_ = nullptr;		// UAV uint x64
	SHEngine::GPUBuffer* pairBuffer_ = nullptr;			// UAV stride8 x64 (つなげないことの確認用)
	std::unique_ptr<SHEngine::ReadBackBuffer> outputReadBack_;
	std::unique_ptr<SHEngine::ComputeTrigger> graphTrigger_;

	FillConfig fillConfig_;
	AddConfig addConfig_;
	bool fireEveryFrame_ = true;
	bool fireTwice_ = false;	// 同一フレームで2回Fireして、CBV書き換えの警告を確認する
	float outputValues_[kGraphTestCount] = {};

private:

	char currentFileName_[256] = "";
	const std::string basePath_ = "Game/StageConfig/";
	const std::string extension_ = ".bin";

	std::vector<std::string> fileList_ = {};

	
};
