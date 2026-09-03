#pragma once
#include <Utility/DataStructures.h>
#include <Tool/Binary/BinaryManager.h>
#include <Camera/Camera.h>
#include <vector>
#include <string>
#include <unordered_map>
#include <map>

using DecoObjData = std::unordered_map<std::string, std::map<int, Transform>>;

namespace Decorate {

	void Save(const DecoObjData& data, BinaryManager& binManager);
	void Load(DecoObjData& data, BinaryManager& binManager);

	struct EditData {
		int id;
		Transform transform;
	};

	class DataManager {
	public:

		DataManager();

		const Transform& GetCurrentTransform() const;

		void EditID(uint32_t id);
		void EditTransform(const Transform& transform, bool correct);
		void AddObject(std::string path, Vector3 position);
		void EraseObject(uint32_t id);

		uint32_t GetCurrentID() const { return currentID_; }
		std::string GetCurrentPath() const { return currentPath_; }
		const DecoObjData& GetObjectInfos() const;
		void SetObjectInfos(const DecoObjData& data);

		void SetCamera(Camera* camera) { camera_ = camera; }

		void Undo();
		void Redo();

		void DrawImGui();

	private:

		enum class HistoryType {
			Transform,
			Erase,
			Add,
			ID,
		};

		struct Data {
			std::string path;
			uint32_t id;
			Transform transform;
		};

		struct ID {
			uint32_t prevID;
			uint32_t nextID;
		};

		struct Trans {
			Transform prevTransform;
			Transform nextTransform;
		};

		void AddHistory(HistoryType type);

		std::string GetPathFromID(uint32_t id) const;

		std::vector<HistoryType> history_;
		int historyIndex_ = -1;

		std::vector<ID> historyID_;
		int historyIDIndex_ = -1;
		std::vector<Trans> historyTransform_;
		int historyTransformIndex_ = -1;
		std::vector<Data> historyErase_;
		int historyEraseIndex_ = -1;
		std::vector<Data> historyAdd_;
		int historyAddIndex_ = -1;

		bool editingTransform_ = false;

		const uint32_t maxHistorySize_ = 128;

		std::unordered_map<std::string, std::map<int, Transform>> transform_;
		uint32_t nextID_ = 1;

		uint32_t currentID_ = 0;
		std::string currentPath_;

		std::string selectedPath_;
		const std::string basePath = "Deco/";

		Camera* camera_ = nullptr;

		const std::string configName = "DecoDataConfig.bin";
	};

}
