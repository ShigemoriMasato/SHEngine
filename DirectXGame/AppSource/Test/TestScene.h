#pragma once
#include <Scene/IScene.h>
#include <Camera/DebugCamera.h>
#include <Tool/Grid/Grid.h>
#include <Render/MeshRenderer.h>
#include <UI/SideBox.h>
#include <Tool/DecoEditor/DecoEditor.h>
#include <Render/PostEffect.h>
#include <Render/Font/Text.h>

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

	std::unique_ptr<ModelDrawer> testModel_;

	std::unique_ptr<PostEffect> postEffect_;
	PostEffectConfig peConfig_;

	std::unique_ptr<SHEngine::Screen::Display> edgeTexture_ = nullptr;
	std::unique_ptr<PostEffect> edgePostEffect_ = nullptr;
	PostEffectConfig edgePeConfig_;

	Grayscale grayScale_;
	Vignette vignette_;
	Blur boxBlur_;
	GaussBlur gaussBlur_;
	Outline outline_;
	RadialBlur radialBlur_;
	Dissolve dissolve_;
	Fade fade_;

	const std::vector<std::string> postEffectNames_ = {
		"None",
		"GrayScale",
		"Vignette",
		"BoxBlur",
		"GaussBlur",
		"EdgeDetection",
		"Outline",
		"RadialBlur",
		"Dissolve",
		"Fade"
	};

	SHEngine::Text text_;
	Transform textTransform_;
	Vector4 textColor_ = { 1.0f, 1.0f, 1.0f, 1.0f };
};
