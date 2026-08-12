#pragma once
#include <Scene/IScene.h>
#include <Camera/DebugCamera.h>
#include <Tool/Grid/Grid.h>
#include <Render/MeshRenderer.h>
#include <UI/SideBox.h>
#include <Tool/DecoEditor/DecoEditor.h>
#include <Render/PostEffect.h>
#include <Render/Font/Text.h>
#include <Tool/ModelDrawer/SkinningProcessor.h>

#include <Game/Effect/Effect.h>
#include <Game/Effect/Wave/WaveEmitter.h>
#include <Game/Effect/Polygon/PolygonEmitter.h>

#include <Game/ParticleTool/IgnoreBallManager.h>

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

	std::unique_ptr<WaveEmitter> waveEmitter_ = nullptr;
	WaveEmitter::Config waveEmitterConfig_ = {};
	WaveEmitter::WaveData waves_ = {};

	std::unique_ptr<PolygonEmitter> polygonEmitter_ = nullptr;
	std::vector<PolygonEmitter::Config> polygonConfigs_ = {};

	ModelDrawer modelDrawer_;
	SkinningProcessor skinningProcessor_;

	Transform modelTransform_ = {};

	PostEffect copy_;
	PostEffectConfig copyConfig_ = {};

	std::vector<std::unique_ptr<ModelDrawer>> modelDrawers_ = {};
};
