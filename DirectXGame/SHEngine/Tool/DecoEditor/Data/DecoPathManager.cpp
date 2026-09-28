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

Decorate::PathManager::PathManager(SHEngine::TextureManager* textureManager, DataManager* dataManager) {
	Update();
	textureManager_ = textureManager;
	dataManager_ = dataManager;

	folder_ = textureManager_->LoadTexture("Assets/.EngineResource/Texture/Folder.png");
	model_ = textureManager_->LoadTexture("Assets/.EngineResource/Texture/Model.png");
}

Decorate::DecoPath Decorate::PathManager::GetDecoPath(const std::string& name) {
	return root_;
}

void Decorate::PathManager::UpdateDecoPath() {
	Update();
}

void Decorate::PathManager::Update() {
	root_ = CreateDirectoryTree("Assets/Model/");
}


void Decorate::PathManager::DrawImGui() {
#ifdef USE_IMGUI

	Decorate::DecoPath* currentNode = &root_;

	std::string currentPath = "Model/";

	for (uint32_t i = 0; i < (uint32_t)currentPath_.size(); ++i) {
		const auto& it = std::find_if(currentNode->children.begin(), currentNode->children.end(),
			[&](const DecoPath& node) {
				return node.name == currentPath_[i];
			});

		if (it != currentNode->children.end()) {
			currentNode = &(*it);
			currentPath += currentPath_[i] + "/";
		} else {
			currentPath_.erase(currentPath_.begin() + i, currentPath_.end());
		}
	}

	ImGui::Begin("Explorer");

	const auto windowSize = ImGui::GetContentRegionAvail();

	ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1), currentPath.c_str());
	const float buttonSize = 96.f;
	const float itemSpacing = 8.0f;
	const float tileWidth = buttonSize + ImGui::GetStyle().FramePadding.x * 2.0f;
	const int columnCount = std::max(1, static_cast<int>((windowSize.x + itemSpacing) / (tileWidth + itemSpacing)));
	int itemIndex = 0;

	auto Button = [&](const SHEngine::TextureData* iconTexture, const std::string& name)->bool {
		if (itemIndex % columnCount != 0) {
			ImGui::SameLine(0.0f, itemSpacing);
		}

		ImGui::BeginGroup();
		ImGui::ImageButton(name.c_str(), iconTexture->GetSRVHandle().ptr, ImVec2(buttonSize, buttonSize));
		const bool doubleClicked = ImGui::IsItemHovered() &&
			ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left);

		const float textWidth = ImGui::CalcTextSize(name.c_str()).x;
		const float textOffset = std::max(0.0f, (tileWidth - textWidth) * 0.5f);
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + textOffset);
		ImGui::TextUnformatted(name.c_str());
		ImGui::EndGroup();

		++itemIndex;

		return doubleClicked;
		};

	SHEngine::TextureData* iconTexture = textureManager_->GetTextureData(folder_);

	//Folder
	for (const auto& child : currentNode->children) {
		if (child.children.empty()) {
			continue;
		}

		if (Button(iconTexture, child.name)) {
			currentPath_.push_back(child.name);
		}
	}

	//Model
	for (const auto& child : currentNode->children) {
		if (!child.children.empty()) {
			continue;
		}
		if (Button(textureManager_->GetTextureData(model_), child.name)) {
			dataManager_->AddObject(child.fullPath, {});
		}
	}

	ImGui::End();

#endif
}
