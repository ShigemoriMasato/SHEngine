#pragma once
#include <Scene/IScene.h>
#include <Camera/DebugCamera.h>
#include <Game/Effect/Effect.h>
#include <Game/Effect/Subject/HitEffect.h>
#include <Render/PostEffect.h>
#include <Tool/TimeViewer/TimeViewer.h>
#include <Game/Effect/Vertex/VertexEmitter.h>
#include <Render/Buffer/ReadBackBuffer.h>
#include <Tool/Grid/Grid.h>

class EffectTestScene : public IScene {
public:

	EffectTestScene() {};

	void Initialize() override;
	std::unique_ptr<IScene> Update() override;
	void Draw() override;

private:

	DebugCamera camera_;

	std::unique_ptr<Effect> effect_ = nullptr;
	std::unique_ptr<VertexEmitter> vertexEmitter_ = nullptr;

	std::unique_ptr<PostEffect> postEffect_ = nullptr;
	PostEffectConfig peConfig_{};

	std::unique_ptr<TimeViewer> timeViewer_ = nullptr;
	std::unique_ptr<SHEngine::ReadBackBuffer> readBackBuffer_ = nullptr;

	std::unique_ptr<Grid> grid_ = nullptr;
};
