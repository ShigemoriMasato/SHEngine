#pragma once
#include <string>
#include <vector>
#include <Assets/Texture/TextureManager.h>

class StageEditor {
public:

	struct FaseData {
		std::string presetName;
		float time = 0.0f;
		float rotation = 0.0f;
	};

	struct StageData {
		std::string name;
		std::vector<FaseData> fases;
		float hpRatio = 1.0f;
		int minEnemyCount = 1;
	};

public:

	void Initialize(SHEngine::TextureManager* textureManager);
	void Update();

	void SetPresetFileList(const std::vector<std::string>& presetFileList) { presetFileList_ = presetFileList; }

	void Save();

private:

	void Load(const std::string& fileName);

	StageData currentStage_;

	std::vector<std::string> presetFileList_;
	std::vector<std::string> stageFileList_;

	const std::string basePath_ = "StageData/";
	const std::string extension_ = ".bin";

	const SHEngine::TextureData* fileTexture_ = nullptr;

	char currentFileName_[256] = "";
};
