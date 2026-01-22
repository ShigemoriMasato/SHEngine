#include "MapEditWithImGui.h"
#include <algorithm>

using namespace MapEditor;

void EditConfig::Initialize() {
}

void EditConfig::DrawImGui(MapDataForBin& mapData) {
#ifdef USE_IMGUI

	ImGui::Begin("Map Config");

	ImGui::Text("Map ID: %d", mapData.mapID);
	ImGui::SameLine();
	if (ImGui::Button("-")) {
		requestMapID_ = std::max(0, mapData.mapID - 1);
	}
	ImGui::SameLine();
	if (ImGui::Button("+")) {
		requestMapID_ = mapData.mapID + 1;
	}

	ImGui::Separator();

	ImGui::InputText("Model File Path", modelFilePathBuffer_, 256);
	if (ImGui::Button("Load")) {
		mapData.modelFilePath = modelFilePathBuffer_;
	}

	int prevWidth = mapData.width;
	if (ImGui::DragInt("Width", &mapData.width, 1.0f, 1, 1024)) {
		int dif = mapData.width - prevWidth;
		if (dif > 0) {
			for (int i = prevWidth; i < mapData.width * mapData.height; i += mapData.width) {
				mapData.tileData.insert(mapData.tileData.begin() + i, dif, TileType::Home);
			}
		} else {
			dif *= -1;
			for (int i = mapData.width; i < mapData.width * mapData.height + 1; i += mapData.width) {
				mapData.tileData.erase(mapData.tileData.begin() + i, mapData.tileData.begin() + i + dif);
			}
		}
	}

	if (ImGui::DragInt("Height", &mapData.height, 1.0f, 1, 1024)) {
		mapData.tileData.resize(mapData.width * mapData.height, TileType::Air);
	}

	ImGui::End();

#endif
}

int EditConfig::RequestMap() {
	return requestMapID_;
}
