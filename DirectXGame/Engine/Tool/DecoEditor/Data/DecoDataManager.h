#pragma once
#include <Utility/DataStructures.h>
#include <vector>
#include <string>
#include <unordered_map>
#include <map>

namespace Decorate {

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
		const std::unordered_map<std::string, std::map<int, Transform>>& GetObjectInfos(std::string path) const;

		void Undo();
		void Redo();

	private:

		enum class HistoryType {
			Waiting,
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

		void AddHistory(HistoryType type);

		std::string GetPathFromID(uint32_t id) const;

		std::vector<HistoryType> history_;

		std::vector<uint32_t> historyID_;
		std::vector<Transform> historyTransform_;
		std::vector<Data> historyErase_;
		std::vector<Data> historyAdd_;

		bool editingTransform_ = false;

		void UpdateHistory();
		void AcceptHistory();

		const uint32_t maxHistorySize_ = 128;

		std::unordered_map<std::string, std::map<int, Transform>> transform_;
		uint32_t nextID_ = 1;

		uint32_t currentID_ = 0;
		std::string currentPath_;

		int historyIndex_ = 0;
	};

}
