#pragma once
#include <Scene/IScene.h>
#include <Camera/DebugCamera.h>
#include <Tool/Grid/Grid.h>

#include <Game/Effect/Effect.h>
#include <Game/Effect/FallPol/FallPolygonEmitter.h>
#include <Game/Effect/RejectPol/RejectBallPolygonEmitter.h>
#include <Game/Effect/FallPol/FallPolygonEmitter.h>

#include <Tool/ModelDrawer/ModelDrawer.h>

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
	Camera orthoCamera_;
	std::unique_ptr<Grid> grid_;

	Effect effect_{};

	FallPolygonEmitter fallEmitter_{};
	RejectBallPolygonEmitter rejectBallEmitter_ = RejectBallPolygonEmitter();

	PolygonEmitter::Config fallConfig_{};
	FallPolygonEmitter::Sphere fallSphere_{};
	RejectBallPolygonEmitter::Config rejectBallConfig_{};
	RejectBallPolygonEmitter::RejectBall rejectBall_{};


	float lifeTime_ = 10.0f;
	Vector3 gravity_ = { 0.0f, -9.8f, 0.0f };

	bool emit_ = false;

	const ModelData* model_ = nullptr;

	ModelDrawer modelDrawer_{};
};
