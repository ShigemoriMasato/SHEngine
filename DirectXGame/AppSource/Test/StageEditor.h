#pragma once
#include <string>
#include <vector>

class StageEditor {
public:

	struct FaseData {
		std::string presetName;
		float time = 0.0f;
	};

	struct StageData {
		std::string name;
		std::vector<FaseData> fases;
		float stageTime = 0.0f;
		float hpRatio = 1.0f;
	};

public:

	void Initialize();
	void Update();

	void SetPresetFileList(const std::vector<std::string>& presetFileList) { presetFileList_ = presetFileList; }

private:

	void Load(const std::string& fileName);
	void Save();

	StageData currentStage_;

	std::vector<std::string> presetFileList_;
	std::vector<std::string> stageFileList_;

	const std::string basePath_ = "StageData/";
	const std::string extension_ = ".bin";
};
