#pragma once
#include <Scene/IScene.h>
#include <SandSimulator/SandSimulator.h>
#include <Camera/DebugCamera.h>
#include <Tool/Grid/Grid.h>

class SandScene : public IScene {
public:

	void Initialize() override;
	std::unique_ptr<IScene> Update() override;
	void Draw() override;

private:

	std::unique_ptr<DebugCamera> camera_;

	std::unique_ptr<SandSimulator> sandSimulator_;

	Grid grid_;

};
