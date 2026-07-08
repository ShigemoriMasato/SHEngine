#pragma once
#include <Scene/IScene.h>
#include <Camera/DebugCamera.h>
#include <Tool/Grid/Grid.h>
#include <Render/MeshRenderer.h>
#include <Game/Effect/Subject/Cylinder.h>

class TestScene : public IScene {
public:

	void Initialize() override;
	std::unique_ptr<IScene> Update() override;
	void Draw() override;

private:

	std::unique_ptr<DebugCamera> debugCamera_;
	std::unique_ptr<Grid> grid_;

	std::unique_ptr<SHEngine::MeshRenderer> meshRenderer_;
	std::unique_ptr<Cylinder> cylinder_ = nullptr;

};
