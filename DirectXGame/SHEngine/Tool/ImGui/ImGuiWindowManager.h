#pragma once
#include <string>
#include <unordered_map>
#include <functional>

class ImGuiWindowManager {
public:

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

	std::unordered_map<std::string, std::function<void()>> drawImGuiList_;
	std::unordered_map<std::string, Branch> confTree_;

};
