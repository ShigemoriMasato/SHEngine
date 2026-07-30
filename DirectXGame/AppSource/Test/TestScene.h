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


	ModelDrawer modelDrawer_;
	SkinningProcessor skinningProcessor_;
};
