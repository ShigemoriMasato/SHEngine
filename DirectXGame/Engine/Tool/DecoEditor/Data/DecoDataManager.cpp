#include "DecoDataManager.h"
#include <cassert>

Decorate::DataManager::DataManager() {
	history_.reserve(maxHistorySize_);
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

void Decorate::DataManager::Begin(HistoryType type, const void* preData) {
	if (currentHistory_.type != HistoryType::Waiting) {
		assert(false && "Invalid history state");
		return;
	}
	currentHistory_.type = type;
	currentHistory_.preData = std::string(reinterpret_cast<const char*>(preData));
}

void Decorate::DataManager::Update(const void* preData) {
	if (currentHistory_.type == HistoryType::Waiting) {
		assert(false && "Invalid history state");
		return;
	}
	currentHistory_.postData = std::string(reinterpret_cast<const char*>(preData));
	UpdateHistory();
}

void Decorate::DataManager::End(const void* postData) {
	currentHistory_.postData = std::string(reinterpret_cast<const char*>(postData));
	AcceptHistory();
}

const std::unordered_map<std::string, std::map<int, Transform>>& Decorate::DataManager::GetObjectInfos(std::string path) const {
	return transform_;
}

void Decorate::DataManager::AddObject(std::string path, Vector3 position) {
	Transform newTransform;
	newTransform.position = position;
	transform_[path][nextID_++] = newTransform;
}

void Decorate::DataManager::EraseObject(int id) {
	if (transform_.find(currentPath_) != transform_.end()) {
		transform_.at(currentPath_).erase(id);
	}
}




void Decorate::DataManager::UpdateHistory() {
	switch(currentHistory_.type) {
		case HistoryType::Transform:
			transform_[currentPath_][currentID_] = *reinterpret_cast<const Transform*>(currentHistory_.postData.data());
			break;
		case HistoryType::ID:
			currentID_ = *reinterpret_cast<const uint32_t*>(currentHistory_.postData.data());
			for (const auto& [path, info] : transform_) {
				if (info.find(currentID_) != info.end()) {
					currentPath_ = path;
					return;
				}
			}
			break;
		default:
			assert(false && "Unknown history type");
			break;
	}
}

void Decorate::DataManager::AcceptHistory() {
	//データ内容が変化していない場合は履歴に追加しない
	if (currentHistory_.preData == currentHistory_.postData) {
		currentHistory_ = {};
		return;
	}

	UpdateHistory();

	history_.push_back(currentHistory_);
	currentHistory_ = {};
	if (history_.size() > maxHistorySize_) {
		history_.erase(history_.begin());
	}
}
