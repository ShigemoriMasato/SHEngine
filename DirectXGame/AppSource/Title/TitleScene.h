#pragma once
#include <Scene/IScene.h>
#include <Title/WaterWave/WaterWave.h>
#include <Render/PostEffect.h>
#include <Game/Effect/Effect.h>

#include <Game/Effect/Polygon/PolygonEmitter.h>

class TitleScene : public IScene {
public:

	TitleScene();
	~TitleScene();

	void Initialize() override;
	std::unique_ptr<IScene> Update() override;
	void Draw() override;

private:

	void Save();
	void Load();

	std::unique_ptr<Camera> camera_ = nullptr;

	std::unique_ptr<WaterWave> waterWave_ = nullptr;

	std::unique_ptr<Effect> effect_ = nullptr;
	std::unique_ptr<PolygonEmitter> polygonEmitter_ = nullptr;
	PolygonEmitter::Config polygonConfig_ = PolygonEmitter::Config(-1);

	PostEffect postEffect_;
	PostEffectConfig peConfig_;

	Fade fade_;


	bool isFadeOut_ = true;
	const float fadeTime_ = 1.0f;
	float timer_ = 0.0f;

	const std::string dataFileName_ = "TitleSceneData.bin";
};
