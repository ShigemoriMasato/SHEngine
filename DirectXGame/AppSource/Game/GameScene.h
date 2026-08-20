#pragma once
#include <Scene/IScene.h>
#include <Game/Tetris/Tetris.h>
#include <Camera/DebugCamera.h>
#include <Render/PostEffect.h>
#include <Game/Camera/GameCamera.h>
#include <Tool/TimeViewer/TimeViewer.h>
#include <Game/Fin/FinScene.h>

#include <Game/Effect/Effect.h>
#include <Game/Effect/Wave/WaveEmitter.h>
#include <Game/Effect/Polygon/PolygonEmitter.h>
#include <Game/Effect/Ellipse/EllipseEmitter.h>
#include <Game/Effect/FallPol/FallPolygonEmitter.h>

#include <Game/ParticleTool/RejectBallManager.h>
#include <Game/ParticleTool/DeleteLineMeshEffect.h>

enum class WaveType {
	Test,
	One,
	Two,
	Three,
	Four,

	Count
};

class GameScene : public IScene {
public:

	GameScene();
	~GameScene() { Save(); }

	void Initialize() override;
	std::unique_ptr<IScene> Update() override;
	void Draw() override;

private:

	void Save();
	void Load();

	Camera* worldCamera_ = nullptr;
	std::unique_ptr<DebugCamera> debugCamera_ = nullptr;
	std::unique_ptr<Camera> manualCamera_ = nullptr;
	std::unique_ptr<GameCamera> gameCamera_ = nullptr;

	std::unique_ptr<TimeViewer> timeViewer_ = nullptr;

	std::unique_ptr<KeyCoating> keyCoating_;
	Tetris tetris_{};
	Effect effect_{};
	std::unique_ptr<FinScene> finScene_{};

	PostEffectConfig postEffectConfig_{};
	PostEffect postEffect_;

	bool prevIsGameOver_ = false;

	// ==========================

	std::unique_ptr<WaveEmitter> waveEmitter_ = nullptr;
	std::unique_ptr<PolygonEmitter> polygonEmitter_ = nullptr;
	std::unique_ptr<RejectBallPolygonEmitter> rejectBallEmitter_ = nullptr;
	std::unique_ptr<FallPolygonEmitter> fallPolygonEmitter_ = nullptr;

	WaveEmitter::Config waveEmitterConfig_ = {};
	std::array<WaveEmitter::WaveData, size_t(WaveType::Count)> waves_ = {};

	RejectBallManager rejectBallManager_ = {};

	DeleteLineMeshEffect deleteLineMeshEffect_;

	Transform tetrisParent_ = {};
};
