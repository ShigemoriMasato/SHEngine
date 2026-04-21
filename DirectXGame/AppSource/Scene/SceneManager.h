#pragma once
#include "IScene.h"

class SceneManager {
public:

	SceneManager() = default;
	~SceneManager() = default;
	void Initialize(SHEngine::Engine* engine);
	void Update();
	void Draw();

	/// @brief アプリケーションの終了命令
	bool IsLoop() const;

private:

	SHEngine::Engine* engine_ = nullptr;
	
	std::unique_ptr<IScene> currentScene_ = nullptr;
	std::unique_ptr<IScene> nextScene_ = nullptr;
	std::unique_ptr<CommonData> commonData_ = nullptr;

};
