#include "ImGuiWindowManager.h"
#include <imgui/imgui.h>

void ImGuiWindowManager::Add(std::function<void()> drawImGui, const std::string& name, std::string group) {
	if(group.empty()) {
		confTree_.emplace(name, Branch{ Conf{false}, {} });
		drawImGuiList_[name] = drawImGui;
		return;
	}

	if (group[group.size() - 1] != '/') {
		group += '/';
	}

	size_t pos = group.find('/');
	size_t prevPos = 0;
	auto branch = &confTree_;
	while (pos != std::string::npos) {
		std::string parentName = group.substr(prevPos, pos - prevPos);
		branch = &(*branch)[parentName].children;
		prevPos = pos + 1;
		pos = group.find('/', pos + 1);
	}

	branch->emplace(name, Branch{ Conf{false}, {} });

	drawImGuiList_[group + name] = drawImGui;
}

void ImGuiWindowManager::DrawImGui() {
#ifdef USE_IMGUI

	//子供がいたらドロップダウン、いなければボタンを表示する
	auto drawBranch = [&](auto&& self, std::unordered_map<std::string, Branch>& branch) -> void {
		for (auto& [name, childBranch] : branch) {
			if (childBranch.children.empty()) {
				if (ImGui::Button(name.c_str())) {
					childBranch.conf.isOpen = true;
				}
				if (childBranch.conf.isOpen) {
					ImGui::SameLine();
					if (ImGui::Button("X")) {
						childBranch.conf.isOpen = false;
					}
				}
			} else {
				if (ImGui::TreeNode(name.c_str())) {
					self(self, childBranch.children);
					ImGui::TreePop();
				}
			}
		}
		};

	ImGui::Begin("Configure");

	drawBranch(drawBranch, confTree_);

	ImGui::End();

	size_t pos = 0;
	size_t prevPos = 0;
	for (auto& [name, drawImGui] : drawImGuiList_) {
		pos = name.find('/');
		prevPos = 0;
		auto* branch = &confTree_;
		if (pos != std::string::npos) {
			branch = &(*branch)[name.substr(prevPos, pos - prevPos)].children;
			prevPos = pos + 1;
			pos = name.find('/', pos + 1);
		}
		std::string leafName = name.substr(prevPos);

		if (!(*branch)[leafName].conf.isOpen) {
			continue;
		}

		ImGui::Begin(name.c_str(), &(*branch)[leafName].conf.isOpen);
		drawImGui();
		ImGui::End();
	}

#endif
}
