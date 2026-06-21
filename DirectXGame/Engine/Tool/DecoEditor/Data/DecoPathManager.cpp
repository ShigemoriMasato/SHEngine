#include "DecoPathManager.h"
#include <filesystem>
#include <imgui/imgui.h>

using namespace Decorate;
namespace fs = std::filesystem;

DecoPath* FindOrCreateChild(std::vector<DecoPath>& children, const std::string& name) {
    auto it = std::find_if(children.begin(), children.end(), 
        [&](const DecoPath& node) {
            return node.name == name;
        });

    if (it != children.end()) {
        return &(*it);
    }

    children.push_back({ name, {} });
    return &children.back();
}

DecoPath CreateDirectoryTree(const fs::path& rootPath) {
    DecoPath root;
    root.name = "Root";

    auto paths = SearchDirectoryPathsAddChild(rootPath);

    for (const auto& pathStr : paths) {
        fs::path path(pathStr);
        DecoPath* current = &root;

        for (const auto& part : path) {
            current = FindOrCreateChild(current->children, part.string());
        }
        current->fullPath = pathStr;
    }

    return root;
}

Decorate::DecoPathManager::DecoPathManager() {
	Update();
}

Decorate::DecoPath Decorate::DecoPathManager::GetDecoPath(const std::string& name) {
	return root_;
}

void Decorate::DecoPathManager::UpdateDecoPath() {
	Update();
}

void Decorate::DecoPathManager::Update() {
	root_ = CreateDirectoryTree("Assets/Model/");
}


void Decorate::DecoPathManager::DrawImGui() {
	DrawDecoPath(root_);
}

void Decorate::DecoPathManager::DrawDecoPath(const DecoPath& node) {
#ifdef USE_IMGUI

    if (node.children.empty()) {

        ImGui::Selectable(node.name.c_str());

        if (ImGui::BeginDragDropSource()) {
            ImGui::SetDragDropPayload("DECO_DROP", node.fullPath.c_str(), node.fullPath.size() + 1);

            ImGui::Text("%s", node.name.c_str());

            ImGui::EndDragDropSource();
        }

    } else {

		if (node.name == "Root") {
            ImGui::SetNextItemOpen(true, ImGuiCond_Once);
		}

        if (ImGui::TreeNode(node.name.c_str())) {

            for (const auto& child : node.children) {
                DrawDecoPath(child);
            }

            ImGui::TreePop();
        }
    }

#endif
}