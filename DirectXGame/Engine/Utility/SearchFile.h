#pragma once
#include <string>
#include <vector>
#include <filesystem>

std::vector<std::string> SearchFiles(const std::filesystem::path& directory, const std::string& extension) {
    std::vector<std::string> contents;

    if (!std::filesystem::exists(directory) || !std::filesystem::is_directory(directory)) {
        return {};
    }

    for (const auto& entry : std::filesystem::directory_iterator(directory)) {
        if (entry.is_regular_file() && entry.path().extension() == extension) {
            std::filesystem::path relativePath = entry.path().lexically_relative(directory);
            contents.push_back(relativePath.generic_string());
        }
    }

    return contents;

}

std::vector<std::string> SearchFileNames(const std::filesystem::path& directory) {
    std::vector<std::string> contents;

    if (!std::filesystem::exists(directory) || !std::filesystem::is_directory(directory)) {
        return {};
    }

    for (const auto& entry : std::filesystem::directory_iterator(directory)) {
        if (entry.is_directory()) {
            contents.push_back(entry.path().filename().generic_string());
        }
    }

    return contents;

}

std::vector<std::string> SerchFilePathsAddChild(const std::filesystem::path& directory, const std::string& extension) {
    std::vector<std::string> result;

    if (!std::filesystem::exists(directory) || !std::filesystem::is_directory(directory)) {
        return {};
    }

    for (const auto& entry : std::filesystem::recursive_directory_iterator(directory)) {
        if (entry.is_regular_file() && (entry.path().extension() == extension || extension.empty())) {
            std::filesystem::path relativePath = entry.path().lexically_relative(directory);
            result.push_back(relativePath.generic_string());
        }
    }

    return result;
}
