#pragma once
#include <Render/Font/Text.h>
#include <SHEngine.h>

class TimeViewer {
public:

	~TimeViewer() { Save(); }

	void Initialize(SHEngine::Engine* engine);

	// @brief 表示する時間を追加する。名前が重複しないこと。重複した場合、上書きされる。
	void Add(std::string name, double time);

	void Delete(std::string name) { texts_.erase(name); }

	// @brief 描画処理
	void Draw(DCC* direct);

	// @brief 描画設定を行う。
	void DrawImGui();

private:

	void Save();
	void Load();

private:

	SHEngine::DrawData drawData_;

	Camera orthoCamera_;

	std::unordered_map<std::string, std::unique_ptr<SHEngine::Text>> texts_;

private:// 描画設定

	float scale_ = 1.0f;
	float interval_ = 30.0f;
	Vector2 offset_ = { 0.0f, 0.0f };
	Vector4 color_ = { 1.0f, 1.0f, 1.0f, 1.0f };

};
