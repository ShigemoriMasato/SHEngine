#pragma once
#include <Scene/IScene.h>
#include <UI/SideBox.h>
#include <Render/PostEffect.h>
#include <Common/KeyConfig/KeyManager.h>

enum class FinSceneUI {
	None = -1,
	Retry,
	Title,

	Count
};

class FinScene {
public:

	FinScene(SHEngine::Engine* engine);

	void PowerOff() { Initialize({}, "", {}); isInitialized_ = false; }

	void Initialize(Vector4 fadeColor, std::string title, Vector4 titleColor);
	FinSceneUI Update(float deltaTime, Vector2 mousePos, std::unordered_map<Key, bool> keys);
	void Draw(DCC* dcc);

	void DrawImGui();

	void Save(BinaryManager& bin);
	void Load(BinaryManager& bin);

private:

	void FadeProcess();
	void UISetup(float deltaTime);
	void UIUpdate(float deltaTime, Vector2 mousePos, std::unordered_map<Key, bool> keys);

private:

	SHEngine::Engine* engine_ = nullptr;

	float timer_ = 0.0f;

private:

	Camera orthoCamera_ = {};

	SHEngine::Text titleText_ = {};
	std::vector<SideBox> sideBoxes_ = {};
	PostEffect backGround_ = {};
	PostEffectConfig backGroundConfig_ = {};
	Fade fade_ = {};

	FinSceneUI currentBox_ = FinSceneUI::None;
	Vector2 prevMousePos_ = {};

private:

	float fadeTime_ = 1.0f;
	float setupTime_ = 0.5f;

	Vector2 titlePos_ = {};
	Vector2 titleScale_ = {};

	float margin_ = 20.0f;
	Vector2 offset_ = { 0.0f, 0.0f };
	Vector2 scale_ = { 1.0f, 1.0f };

	std::vector<Vector4> colorMap_;
	bool isInitialized_ = false;


	std::vector<std::string> textMap_ = { "Retry", "Title" };
};
