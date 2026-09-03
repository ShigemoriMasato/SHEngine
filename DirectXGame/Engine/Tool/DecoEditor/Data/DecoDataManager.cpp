#include "DecoDataManager.h"
#include <imgui/imgui.h>
#include <cassert>
#include <Tool/Binary/BinaryManager.h>
#include <Utility/SearchFile.h>

Decorate::DataManager::DataManager() {
	if (!std::filesystem::exists("Assets/Binary/" + basePath)) {
		std::filesystem::create_directories("Assets/Binary/" + basePath);
	}

	BinaryManager fileNameManager;
	if (!fileNameManager.Boot(configName)) {
		return;
	}
	selectedPath_ = fileNameManager.Reverse<std::string>();
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
	constexpr static float cameraDistance = 20.0f;
	Vector3 pos = position;
	pos += camera_->GetPosition() + camera_->GetDirection() * cameraDistance;

	pos.y = 0.0f;

	AddHistory(HistoryType::Add);
	Data data{};
	data.path = path;
	data.id = nextID_++;
	data.transform.position = pos;

	transform_[data.path][data.id] = data.transform;
	historyAdd_.push_back(data);
	historyAddIndex_++;
}

void Decorate::DataManager::EraseObject(uint32_t id) {
	Data data{};
	data.path = GetPathFromID(id);
	if (data.path.empty()) {
		return;
	}
	AddHistory(HistoryType::Erase);
	data.id = id;
	data.transform = transform_.at(data.path).at(data.id);

	historyErase_.push_back(data);
	historyEraseIndex_++;

	transform_.at(data.path).erase(data.id);

	currentID_ = 0;
}

const std::unordered_map<std::string, std::map<int, Transform>>& Decorate::DataManager::GetObjectInfos() const {
	return transform_;
}

void Decorate::DataManager::SetObjectInfos(const DecoObjData& data) {
	for (auto& [path, infos] : transform_) {
		infos.clear();
	}

	transform_ = data;

	for (const auto& [path, infos] : transform_) {
		for (const auto& [id, transform] : infos) {
			if (id >= int(nextID_)) {
				nextID_ = id + 1;
			}
		}
	}

	history_.clear();
	historyAdd_.clear();
	historyErase_.clear();
	historyTransform_.clear();
	historyID_.clear();

	historyIndex_ = -1;
	historyAddIndex_ = -1;
	historyEraseIndex_ = -1;
	historyTransformIndex_ = -1;
	historyIDIndex_ = -1;

	currentPath_.clear();
	currentID_ = 0;
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

void Decorate::Save(const DecoObjData& data, BinaryManager& binManager) {
	const static std::string key = "DecoObjData";
	binManager.Register(&key);

	uint32_t pathCount = static_cast<uint32_t>(data.size());
	binManager.Register(&pathCount);

	for (const auto& [path, transforms] : data) {
		binManager.Register(&path);

		uint32_t transformCount = static_cast<uint32_t>(transforms.size());
		binManager.Register(&transformCount);

		for (const auto& [id, transform] : transforms) {
			binManager.Register(&transform);
		}
	}
}

void Decorate::Load(DecoObjData& data, BinaryManager& binManager) {
	const static std::string key = "DecoObjData";
	std::string readKey = binManager.Reverse<std::string>();

	int nextID = 1;

	if (readKey != key) {
		binManager.Back();
		return;
	}

	uint32_t pathCount = binManager.Reverse<uint32_t>();

	for (uint32_t i = 0; i < pathCount; ++i) {
		std::string path = binManager.Reverse<std::string>();
		uint32_t transformCount = binManager.Reverse<uint32_t>();

		for (uint32_t j = 0; j < transformCount; ++j) {
			Transform transform = binManager.Reverse<Transform>();
			data[path][nextID++] = transform;
		}
	}
}
