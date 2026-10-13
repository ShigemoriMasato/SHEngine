#pragma once
#include <Scene/IScene.h>
#include <SandSimulator/SandSimulator.h>
#include <SandSimulator/CPUSample/sandCPUSample.h>
#include <Camera/DebugCamera.h>
#include <Tool/Grid/Grid.h>

class SandScene : public IScene {
public:

	~SandScene();

	void Initialize() override;
	std::unique_ptr<IScene> Update() override;
	void Draw() override;

private:

	void Save() const;
	void Load();

	std::unique_ptr<DebugCamera> camera_;

	std::unique_ptr<SandSimulator> sandSimulator_;

	Grid grid_;

	std::unique_ptr<SandCPUSample> sandCPUSample_;

};
