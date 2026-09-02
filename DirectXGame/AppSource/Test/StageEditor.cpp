#include "StageEditor.h"
#include <Utility/SearchFile.h>
#include <imgui/imgui.h>

void StageEditor::Initialize() {
	if (!IsBeFile("Assets/Binary/" + basePath_)) {
		std::filesystem::create_directories("Assets/Binary/" + basePath_);
	}

	auto files = SearchFiles("Assets/Binary/" + basePath_, ".bin");

	for (const auto& filePath : files) {
		std::filesystem::path path(filePath);
		std::string fileName = path.stem().string();
		stageFileList_.push_back(fileName);
	}
}

void StageEditor::Update() {
#ifdef USE_IMGUI

	if (presetFileList_.empty()) {
		return;
	}

	std::vector<const char*> presetFileListCStr;
	presetFileListCStr.reserve(presetFileList_.size());
	for (const auto& presetName : presetFileList_) {
		presetFileListCStr.push_back(presetName.c_str());
	}

	ImGui::Begin("Stage Editor");

	for (uint32_t i = 0; i < currentStage_.fases.size(); ++i) {
		int currentFaseIndex = 0;
		for (int j = 0; j < int(presetFileList_.size()); ++j) {
			if (currentStage_.fases[i].presetName == presetFileList_[j]) {
				currentFaseIndex = j;
				break;
			}
		}

		ImGui::PushID(i);

		if (ImGui::Button("-")) {
			currentStage_.fases.erase(currentStage_.fases.begin() + i);
			i--;
			ImGui::PopID();
			continue;
		}

		ImGui::SameLine();

		ImGui::Combo("Preset", &currentFaseIndex, presetFileListCStr.data(), static_cast<int>(presetFileListCStr.size()));
		currentStage_.fases[i].presetName = presetFileList_[currentFaseIndex];

		ImGui::DragFloat("Time", &currentStage_.fases[i].time, 0.1f, 0.0f);

		ImGui::PopID();

		ImGui::Separator();
	}

	if (ImGui::Button("+")) {
		currentStage_.fases.push_back(FaseData());
	}

	ImGui::End();

#endif
}
