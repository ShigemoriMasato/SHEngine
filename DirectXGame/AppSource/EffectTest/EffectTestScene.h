#pragma once
#include <Scene/IScene.h>
#include <Camera/DebugCamera.h>
#include <Game/Effect/Effect.h>
#include <Render/PostEffect.h>
#include <Tool/TimeViewer/TimeViewer.h>
#include <Render/Buffer/ReadBackBuffer.h>
#include <Tool/Grid/Grid.h>

#include <Game/Effect/Vertex/VertexEmitter.h>
#include <Game/Effect/Polygon/PolygonEmitter.h>
#include <Game/Effect/Wave/WaveEmitter.h>

class EffectTestScene : public IScene {
public:

	EffectTestScene() { Load(); };
	~EffectTestScene() { Save(); };

	void Initialize() override;
	std::unique_ptr<IScene> Update() override;
	void Draw() override;

private:

	void Save();
	void Load();

	DebugCamera camera_;

	std::unique_ptr<SHEngine::BufferContainer> container_ = nullptr;

	std::unique_ptr<Effect> effect_ = nullptr;
	std::unique_ptr<VertexEmitter> vertexEmitter_ = nullptr;
	std::unique_ptr<PolygonEmitter> polygonEmitter_ = nullptr;

	std::unique_ptr<WaveEmitter> waveEmitter_ = nullptr;
	WaveEmitter::Config waveEmitterConfig_{};
	WaveEmitter::WaveData waveData_{};

	std::unique_ptr<PostEffect> postEffect_ = nullptr;
	PostEffectConfig peConfig_{};

	std::unique_ptr<TimeViewer> timeViewer_ = nullptr;

	std::unique_ptr<Grid> grid_ = nullptr;

	bool drawGrid_ = false;

	const std::string savefile_ = "EffectTestScene.bin";
};
