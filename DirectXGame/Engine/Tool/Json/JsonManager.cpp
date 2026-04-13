#include "JsonManager.h"
#include <fstream>
#include <filesystem>

void JsonManager::Boot(const std::string& fileName) {
	std::filesystem::create_directories(directory);
	jsonData_.clear();
	fileName_ = fileName;
	std::ifstream file(std::string(directory + fileName + extension));
	if (!file.is_open()) {
		return;
	}
	file >> jsonData_;
}

void JsonManager::Save() {
	std::ofstream file(std::string(directory + fileName_ + extension));
	if (!file.is_open()) {
		return;
	}
	file << jsonData_.dump(4);
}
