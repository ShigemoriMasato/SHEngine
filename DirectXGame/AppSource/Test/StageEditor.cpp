#include "StageEditor.h"
#include <algorithm>
#include <Utility/SearchFile.h>
#include <Tool/Binary/BinaryManager.h>
#include <imgui/imgui.h>

void StageEditor::Initialize(SHEngine::TextureManager* textureManager) {
	if (!IsBeFile("Assets/Binary/" + basePath_)) {
		std::filesystem::create_directories("Assets/Binary/" + basePath_);
	}

	auto files = SearchFiles("Assets/Binary/" + basePath_, ".bin");

	for (const auto& filePath : files) {
		std::filesystem::path path(filePath);
		std::string fileName = path.stem().string();
		stageFileList_.push_back(fileName);
	}

	fileTexture_ = textureManager->GetTextureData("Assets/.EngineResource/Texture/File.png");
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

	auto windowSize = ImGui::GetContentRegionAvail();

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

		ImGui::PushItemWidth(windowSize.x / 2.0f - 50.0f);
		ImGui::DragFloat("Time", &currentStage_.fases[i].time, 0.1f, 0.0f);
		ImGui::SameLine();
		ImGui::DragFloat("Rotation", &currentStage_.fases[i].rotation, 0.01f);
		ImGui::PopItemWidth();

		ImGui::PopID();

		ImGui::Separator();
	}

	if (ImGui::Button("+")) {
		currentStage_.fases.push_back(FaseData());
	}

	ImGui::End();

	ImGui::Begin("ステージ");

	ImGui::InputText("File Name", currentFileName_, sizeof(currentFileName_));
	if (ImGui::Button("+")) {
		currentStage_.name = currentFileName_;
		Save();
	}

	windowSize = ImGui::GetContentRegionAvail();

	const float buttonSize = 64.f;
	const float itemSpacing = 8.0f;
	const float tileWidth = buttonSize + ImGui::GetStyle().FramePadding.x * 2.0f;
	const int columnCount = std::max(1, static_cast<int>((windowSize.x + itemSpacing) / (tileWidth + itemSpacing)));
	int itemIndex = 0;

	auto Button = [&](const SHEngine::TextureData* iconTexture, const std::string& name)->bool {
		if (itemIndex % columnCount != 0) {
			ImGui::SameLine(0.0f, itemSpacing);
		}

		ImGui::BeginGroup();
		ImGui::ImageButton(name.c_str(), iconTexture->GetSRVHandle().ptr, ImVec2(buttonSize, buttonSize));
		const bool doubleClicked = ImGui::IsItemHovered() &&
			ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left);

		const float textWidth = ImGui::CalcTextSize(name.c_str()).x;
		const float textOffset = std::max(0.0f, (tileWidth - textWidth) * 0.5f);
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + textOffset);
		ImGui::TextUnformatted(name.c_str());
		ImGui::EndGroup();

		++itemIndex;

		return doubleClicked;
		};


	for (const auto& fileName : stageFileList_) {
		if (Button(fileTexture_, fileName)) {
			Save();

			Load(fileName);
		}
	}

	ImGui::End();

	ImGui::Begin("Stage Settings");
	ImGui::DragFloat("HP倍率", &currentStage_.hpRatio, 0.01f, 0.0f);
	ImGui::DragInt("最小敵数", &currentStage_.minEnemyCount, 1, 1);
	ImGui::End();

#endif
}

void StageEditor::Load(const std::string& fileName) {
	BinaryManager bin;
	if (!bin.Boot(basePath_ + fileName + extension_)) {
		return;
	}
	currentStage_.name = fileName;
	int faseCount = bin.Reverse<int>();
	currentStage_.fases.resize(faseCount);
	for (auto& fase : currentStage_.fases) {
		fase.presetName = bin.Reverse<std::string>();
		fase.time = bin.Reverse<float>();
		fase.rotation = bin.Reverse<float>();
	}

	currentStage_.hpRatio = bin.Reverse<float>();
	currentStage_.minEnemyCount = bin.Reverse<int>();
}

void StageEditor::Save() {
	if (currentStage_.name.empty()) {
		return;
	}

	const auto& it = std::find(stageFileList_.begin(), stageFileList_.end(), currentStage_.name);
	if (it == stageFileList_.end()) {
		stageFileList_.push_back(currentStage_.name);
	}

	BinaryManager bin;
	
	int faseCount = static_cast<int>(currentStage_.fases.size());
	bin.Register(&faseCount);
	for (const auto& fase : currentStage_.fases) {
		bin.Register(&fase.presetName);
		bin.Register(&fase.time);
		bin.Register(&fase.rotation);
	}

	bin.Register(&currentStage_.hpRatio);
	bin.Register(&currentStage_.minEnemyCount);

	bin.Write(basePath_ + currentStage_.name + extension_);
}
