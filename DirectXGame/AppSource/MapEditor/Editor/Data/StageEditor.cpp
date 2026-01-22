#include "StageEditor.h"
#include <imgui/imgui.h>

using namespace MapEditor;

namespace {
#ifdef USE_IMGUI
	bool ToggleInt(std::string tag, int& num, int id, int min = 0, int max = 100) {
		ImGui::PushID(id);
		bool changed = false;
		ImGui::Text("%s: %d", tag.c_str(), num);
		ImGui::SameLine();
		if (ImGui::Button("-")) {
			changed = true;
			num = std::max(min, num - 1);
		}
		ImGui::SameLine();
		if (ImGui::Button("+")) {
			changed = true;
			num = std::min(max, num + 1);
		}
		ImGui::PopID();
		return changed;
	}
#endif
}

void StageEditor::Initialize() {
	stageData_.reserve(16);
	Load();
	if (stageData_.empty()) {
		stageData_.emplace_back();
	}
	numberOfStages_ = static_cast<int>(stageData_.size());

	directionNames_ = {
		"Front",
		"Right",
		"Back",
		"Left"
	};
}

void StageEditor::DrawImGui(const std::vector<MapDataForBin>& mapData, std::map<TileType, ImU32> tileColorMap) {
#ifdef USE_IMGUI

	if (mapData.empty()) {
		return;
	}

	ImGui::Begin("Stage Editor");

	if (ToggleInt("Number of Stages", numberOfStages_, 0, 1)) {
		stageData_.resize(numberOfStages_);
	}
	ToggleInt("Edit Stage ID", editStageID_, 1, 0, numberOfStages_ - 1);

	ImGui::BeginChild("StageConfig", { 300, 0 }, false);

	StageData& editStage = stageData_[editStageID_];
	ToggleInt("MapID", editStage.mapID, 0, 0, int(mapData.size() - 1));
	ImGui::Combo("Direction", (int*)&editStage.direction, directionNames_.data(), (int)directionNames_.size());
	ImGui::DragFloat("Gold Frequency", &editStage.goldFrequency, 0.01f, 0.0f, 1.0f);
	ImGui::EndChild();

	ImGui::SameLine();

	//todo マップのプレビューを表示する
	ImGui::BeginChild("Preview", {}, true);
	ImVec2 windowSize = ImGui::GetWindowSize();
	const MapDataForBin& map = mapData[editStage.mapID];

	auto stage = map.GetDirectionGrid(editStage.direction);
	auto drawList = ImGui::GetWindowDrawList();
	ImVec2 origin = ImGui::GetCursorScreenPos();

	if (!stage.empty() && !stage[0].empty()) {
		float size = 0;
		{
			float width = windowSize.x / (float)stage[0].size();
			float height = windowSize.y / (float)stage.size();
			size = std::min(width, height);
		}

		for (int i = 0; i < stage.size(); ++i) {
			for (int j = 0; j < stage[i].size(); ++j) {
				ImVec2 min(origin.x + j * size, origin.y + i * size);
				ImVec2 max(min.x + size, min.y + size);

				drawList->AddRectFilled(min, max, tileColorMap[stage[i][j]]);
			}
		}
	}

	ImGui::EndChild();

	ImGui::End();

#endif
}

void StageEditor::Save() {
	binaryManager_.RegisterOutput(int(stageData_.size()));
	for (const auto& stage : stageData_) {
		binaryManager_.RegisterOutput(stage.stageID);
		binaryManager_.RegisterOutput(stage.mapID);
		binaryManager_.RegisterOutput(int(stage.direction));
		binaryManager_.RegisterOutput(stage.goldFrequency);
	}

	binaryManager_.Write(stageSaveFile_);
}

void StageEditor::Load() {
	auto values = binaryManager_.Read(stageSaveFile_);
	if (values.empty()) {
		return;
	}

	size_t index = 0;
	int stageCount = BinaryManager::Reverse<int>(values[index++].get());
	stageData_.resize(stageCount);
	for (int i = 0; i < stageCount; ++i) {
		stageData_[i].stageID = BinaryManager::Reverse<int>(values[index++].get());
		stageData_[i].mapID = BinaryManager::Reverse<int>(values[index++].get());
		stageData_[i].direction = (Direction)BinaryManager::Reverse<int>(values[index++].get());
		stageData_[i].goldFrequency = BinaryManager::Reverse<float>(values[index++].get());
	}
}
