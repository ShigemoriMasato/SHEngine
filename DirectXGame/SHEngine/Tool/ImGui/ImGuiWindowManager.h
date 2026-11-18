#pragma once
#include <string>
#include <unordered_map>
#include <functional>

class ImGuiWindowManager {
public:

	// @brief 今後追加されるアイテムをこのグループに所属させる。
	void PushGroup(const std::string& groupName);
	// @brief ひとつ前のグループに戻る。グループがない場合は何もしない。
	void PopGroup();

	//Begin,EndをのぞいたImGuiの関数
	void Add(std::function<void()> drawImGui, const std::string& name, std::string group = "");

	void DrawImGui();

private:

	struct Conf {
		bool isOpen = false;
	};

	struct Branch {
		Conf conf;
		std::unordered_map<std::string, Branch> children;
	};

	std::vector<std::string> groupStack_;

	std::unordered_map<std::string, std::function<void()>> drawImGuiList_;
	std::unordered_map<std::string, Branch> confTree_;

};
