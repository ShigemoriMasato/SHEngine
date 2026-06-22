#include "DecoDataManager.h"
#include <cassert>

Decorate::DataManager::DataManager() {
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

	currentID_ = id;
	currentPath_ = GetPathFromID(id);
	history_.push_back(HistoryType::ID);
	historyID_.push_back(id);
}

void Decorate::DataManager::EditTransform(const Transform& transform, bool correct) {
	const Transform& currentTransform = GetCurrentTransform();

	if (!editingTransform_) {
		history_.push_back(HistoryType::Transform);
		historyTransform_.push_back(transform);
		editingTransform_ = true;
	}

	historyTransform_.back() = transform;

	if (correct) {
		editingTransform_ = false;
	}
}

void Decorate::DataManager::AddObject(std::string path, Vector3 position) {
	history_.push_back(HistoryType::Add);
	Data data{};
	data.path = path;
	data.id = nextID_++;
	data.transform.position = position;

	transform_[data.path][data.id] = data.transform;
	historyAdd_.push_back(data);
}

void Decorate::DataManager::EraseObject(uint32_t id) {
	history_.push_back(HistoryType::Erase);
	Data data{};
	data.path = GetPathFromID(id);
	data.id = id;
	data.transform = transform_.at(data.path).at(data.id);

	historyErase_.push_back(data);

	transform_.at(data.path).erase(data.id);
}

const std::unordered_map<std::string, std::map<int, Transform>>& Decorate::DataManager::GetObjectInfos(std::string path) const {
	return transform_;
}

void Decorate::DataManager::Undo() {

}

void Decorate::DataManager::Redo() {
	
}

void Decorate::DataManager::AddHistory(HistoryType type) {
	uint32_t currentHistorySize = static_cast<uint32_t>(history_.size());
	if (historyIndex_ < currentHistorySize - 1) {
		history_.resize(historyIndex_ + 1);
		historyID_.resize(historyIndex_ + 1);
		historyTransform_.resize(historyIndex_ + 1);
		historyErase_.resize(historyIndex_ + 1);
		historyAdd_.resize(historyIndex_ + 1);
	}
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
