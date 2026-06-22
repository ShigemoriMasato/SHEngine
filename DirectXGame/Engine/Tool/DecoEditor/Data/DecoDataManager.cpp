#include "DecoDataManager.h"
#include <imgui/imgui.h>
#include <cassert>

Decorate::DataManager::DataManager() {
	EditID(0);
}

const Transform& Decorate::DataManager::GetCurrentTransform() const {
	static Transform defaultTransform{};
	if (currentPath_.empty()) {
		return defaultTransform;
	}
	if (transform_.at(currentPath_).find(currentID_) == transform_.at(currentPath_).end()) {
		return defaultTransform;
	}

	return transform_.at(currentPath_).at(currentID_);
}

void Decorate::DataManager::EditID(uint32_t id) {
	if (currentID_ == id) {
		return;
	}

	historyID_.push_back({currentID_, id});
	historyIDIndex_++;
	currentID_ = id;
	currentPath_ = GetPathFromID(id);
	AddHistory(HistoryType::ID);
}

void Decorate::DataManager::EditTransform(const Transform& transform, bool correct) {
	const Transform& currentTransform = GetCurrentTransform();

	if (!editingTransform_) {
		AddHistory(HistoryType::Transform);
		historyTransform_.push_back({ currentTransform, transform });
		historyTransformIndex_++;
		editingTransform_ = true;
	}

	historyTransform_[historyTransform_.size() - 1].nextTransform = transform;
	transform_[currentPath_][currentID_] = transform;

	if (correct) {
		editingTransform_ = false;
	}
}

void Decorate::DataManager::AddObject(std::string path, Vector3 position) {
	AddHistory(HistoryType::Add);
	Data data{};
	data.path = path;
	data.id = nextID_++;
	data.transform.position = position;

	transform_[data.path][data.id] = data.transform;
	historyAdd_.push_back(data);
	historyAddIndex_++;
}

void Decorate::DataManager::EraseObject(uint32_t id) {
	AddHistory(HistoryType::Erase);
	Data data{};
	data.path = GetPathFromID(id);
	data.id = id;
	data.transform = transform_.at(data.path).at(data.id);

	historyErase_.push_back(data);
	historyEraseIndex_++;

	transform_.at(data.path).erase(data.id);
}

const std::unordered_map<std::string, std::map<int, Transform>>& Decorate::DataManager::GetObjectInfos(std::string path) const {
	return transform_;
}

void Decorate::DataManager::Undo() {
	if (historyIndex_ < 0) {
		return;
	}

	HistoryType type = history_[historyIndex_];
	historyIndex_--;

	switch (type) {
	case HistoryType::Transform:
	{
		if (historyTransformIndex_ < 0) {
			break;
		}
		transform_[currentPath_][currentID_] = historyTransform_[historyTransformIndex_].prevTransform;
		historyTransformIndex_--;
		break;
	}
	case HistoryType::Erase:
	{
		if (historyEraseIndex_ < 0) {
			break;
		}
		const Data& data = historyErase_[historyEraseIndex_];
		transform_[data.path][data.id] = data.transform;
		historyEraseIndex_--;
		break;
	}
	case HistoryType::Add:
	{
		if (historyAddIndex_ < 0) {
			break;
		}
		const Data& data = historyAdd_[historyAddIndex_];
		transform_.at(data.path).erase(data.id);
		historyAddIndex_--;
		break;
	}
	case HistoryType::ID:
	{
		if (historyIDIndex_ < 0) {
			break;
		}
		currentID_ = historyID_[historyIDIndex_].prevID;
		currentPath_ = GetPathFromID(currentID_);
		historyIDIndex_--;
		break;
	}
	}
}

void Decorate::DataManager::Redo() {
	if (historyIndex_ >= static_cast<int>(history_.size()) - 1) {
		return;
	}

	historyIndex_++;
	HistoryType type = history_[historyIndex_];

	switch (type) {
	case HistoryType::Transform:
	{
		historyTransformIndex_++;
		transform_[currentPath_][currentID_] = historyTransform_[historyTransformIndex_].nextTransform;
		break;
	}
	case HistoryType::Erase:
	{
		historyEraseIndex_++;
		const Data& data = historyErase_[historyEraseIndex_];
		transform_.at(data.path).erase(data.id);
		break;
	}
	case HistoryType::Add:
	{
		historyAddIndex_++;
		const Data& data = historyAdd_[historyAddIndex_];
		transform_[data.path][data.id] = data.transform;
		break;
	}
	case HistoryType::ID:
	{
		historyIDIndex_++;
		currentID_ = historyID_[historyIDIndex_].nextID;
		currentPath_ = GetPathFromID(currentID_);
		break;
	}
	}
}

void Decorate::DataManager::DrawImGui() {
#ifdef USE_IMGUI

	ImGui::Begin("DataManager");

	ImGui::Text("Transform: %d, %d", int(historyTransform_.size()), historyTransformIndex_);
	ImGui::Text("Erase: %d, %d", int(historyErase_.size()), historyEraseIndex_);
	ImGui::Text("Add: %d, %d", int(historyAdd_.size()), historyAddIndex_);
	ImGui::Text("ID: %d, %d", int(historyID_.size()), historyIDIndex_);

	ImGui::End();

#endif // USE_IMGUI

}

void Decorate::DataManager::AddHistory(HistoryType type) {
	int currentHistorySize = static_cast<int>(history_.size());

	//過去にRedoした履歴がある場合は削除する
	if (historyIndex_ < currentHistorySize - 1) {
		history_.resize(historyIndex_ + 1);
		historyAdd_.resize(historyAddIndex_ + 1);
		historyErase_.resize(historyEraseIndex_ + 1);
		historyTransform_.resize(historyTransformIndex_ + 1);
		historyID_.resize(historyIDIndex_ + 1);
	}

	history_.push_back(type);
	if ((uint32_t)history_.size() > maxHistorySize_) {
		switch(history_.front()) {
		case Decorate::DataManager::HistoryType::Transform:
			historyTransform_.erase(historyTransform_.begin());
			break;
		case Decorate::DataManager::HistoryType::Erase:
			historyErase_.erase(historyErase_.begin());
			break;
		case Decorate::DataManager::HistoryType::Add:
			historyAdd_.erase(historyAdd_.begin());
			break;
		case Decorate::DataManager::HistoryType::ID:
			historyID_.erase(historyID_.begin());
			break;
		}
		history_.erase(history_.begin());
	}
	historyIndex_ = int(history_.size()) - 1;
}

std::string Decorate::DataManager::GetPathFromID(uint32_t id) const {
	for (const auto& [path, info] : transform_) {
		if (info.find(id) != info.end()) {
			return path;
		}
	}
	return std::string();
}



void Decorate::DataManager::UpdateHistory() {
	
}

void Decorate::DataManager::AcceptHistory() {
	//データ内容が変化していない場合は履歴に追加しない
	
}
