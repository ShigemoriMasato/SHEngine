#pragma once
#include <string>
#include <vector>
#include <filesystem>

//指定したディレクトリ内から、特定の拡張子と一致するファイルを探索する。ディレクトリは除外
std::vector<std::string> SearchFiles(const std::filesystem::path& directory, const std::string& extension);

//指定したディレクトリ内のサブディレクトリ名を取得する。ファイルは除外
std::vector<std::string> SearchDirectoryNames(const std::filesystem::path& directory);

// 指定したディレクトリ内から、特定の拡張子と一致するファイルのパスを検索する。ディレクトリがある場合、その中も再帰的に探索する
std::vector<std::string> SearchFilePathsAddChild(const std::filesystem::path& directory, const std::string& extension);

// 指定したディレクトリ内のサブディレクトリのパスを再帰的に取得する。ファイルは除外
std::vector<std::string> SearchDirectoryPathsAddChild(const std::filesystem::path& directory);

//ファイルが存在するかどうか
bool IsBeFile(const std::string& filePath);
