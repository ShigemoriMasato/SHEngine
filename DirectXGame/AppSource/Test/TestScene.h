#pragma once
#include <Scene/IScene.h>
#include "DDS/DDSTest.h"
#include <Camera/DebugCamera.h>
#include <Tool/Grid/Grid.h>

class TestScene : public IScene {
public:

	void Initialize() override;
	std::unique_ptr<IScene> Update() override;
	void Draw() override;

private:

	std::unique_ptr<DebugCamera> debugCamera_;
	std::unique_ptr<Grid> grid_;

	std::unique_ptr<DDSTest> ddsTest_;

};
