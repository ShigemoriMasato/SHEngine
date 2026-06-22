#pragma once
#include <Utility/DataStructures.h>
#include <vector>
#include <string>
#include <unordered_map>
#include <map>

namespace Decorate {

	enum class HistoryType {
		Waiting,
		Transform,
		ID,
	};

	struct EditData {
		int id;
		Transform transform;
	};

	class DataManager {
	public:

		DataManager();

		const Transform& GetCurrentTransform() const;

		void Begin(HistoryType type, const void* preData);
		void Update(const void* preData);
		void End(const void* postData);

		uint32_t GetCurrentID() const { return currentID_; }
		const std::unordered_map<std::string, std::map<int, Transform>>& GetObjectInfos(std::string path) const;

		void AddObject(std::string path, Vector3 position);
		void EraseObject(int id);

	private:

		void UpdateHistory();
		void AcceptHistory();

		struct HistoryData {
			HistoryType type = HistoryType::Waiting;
			std::string preData;
			std::string postData;
		};

		HistoryData currentHistory_;
		std::vector<HistoryData> history_;
		const uint32_t maxHistorySize_ = 128;

		std::unordered_map<std::string, std::map<int, Transform>> transform_;
		uint32_t nextID_ = 1;

		uint32_t currentID_ = 0;
		std::string currentPath_;
	};

}
