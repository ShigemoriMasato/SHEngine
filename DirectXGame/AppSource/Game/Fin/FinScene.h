#pragma once
#include <Scene/IScene.h>
#include <UI/SideBox.h>
#include <Render/PostEffect.h>

enum class FinSceneUI {
	None = -1,
	Retry,
	Title,

	Count
};

class FinScene {
public:

	FinScene(SHEngine::Engine* engine, SHEngine::Screen::Display* display);

	void PowerOff() { Initialize({}, "", {}); isInitialized_ = false; }

	void Initialize(Vector4 fadeColor, std::string title, Vector4 titleColor);
	FinSceneUI Update(float deltaTime, Vector2 mousePos);
	//内部ディスプレイへ描画
	void DrawReady(DCC* dcc);
	//内部ディスプレイを現在のRenderTargetへ描画
	void Draw(DCC* dcc);

	void DrawImGui();

	void Save(BinaryManager& bin);
	void Load(BinaryManager& bin);

private:

	void FadeProcess();
	void UISetup(float deltaTime);
	void UIUpdate(float deltaTime);

private:

	SHEngine::Engine* engine_ = nullptr;

	float fadeTime_ = 1.0f;
	float setupTime_ = 0.5f;

	float timer_ = 0.0f;

private:

	//
	SHEngine::Screen::Display* display_ = nullptr;
	Camera orthoCamera_ = {};

	SHEngine::Text titleText_ = {};
	std::vector<SideBox> sideBoxes_ = {};
	PostEffect backGround_ = {};
	PostEffectConfig backGroundConfig_ = {};
	Fade fade_ = {};

	PostEffect lastCopy_ = {};
	PostEffectConfig lastCopyConfig_ = {};

	FinSceneUI currentBox_ = FinSceneUI::None;
	Vector2 prevMousePos_ = {};

private:

	Vector2 titlePos_ = {};
	Vector2 titleScale_ = {};

	float margin_ = 20.0f;
	Vector2 offset_ = { 0.0f, 0.0f };
	Vector2 scale_ = { 1.0f, 1.0f };

	std::vector<Vector4> colorMap_;
	bool isInitialized_ = false;


	std::vector<std::string> textMap_ = { "Retry", "Title" };
};
