#include "Campus.h"
#include <imgui/imgui.h>
#include <Utility/Color.h>
#include <Input/Input.h>

using namespace MapEditor;

void Campus::Initialize() {
	for (int i = 0; i < static_cast<int>(TileType::Count); ++i) {
		tileColorMap_[static_cast<TileType>(i)] = IM_COL32(100 + i * 30, 100 + i * 20, 200 - i * 20, 255);
	}

	Load();
}

void MapEditor::Campus::DrawImGui(MapDataForBin& data) {
#ifdef USE_IMGUI

	if (preMapID_ != data.mapID) {
		undoStack_.clear();
		preMapID_ = data.mapID;
	}

	ImGui::Begin("Campus");
	
	ImGui::BeginChild(
		"CampusScroll",
		ImVec2(0, 0),
		true,
		ImGuiWindowFlags_HorizontalScrollbar
	);
	
	ImVec2 mapSize(
		float(data.width * tileSize_),
		float(data.height * tileSize_)
	);

	ImGui::InvisibleButton("MapCanvas", mapSize);

	ImDrawList* drawList = ImGui::GetWindowDrawList();
	ImVec2 origin = ImGui::GetItemRectMin();

	for (int i = 0; i < data.height; ++i) {
		for (int j = 0; j < data.width; ++j) {
			int index = i * data.width + j;

			ImVec2 min(origin.x + j * tileSize_, origin.y + i * tileSize_);
			ImVec2 max(min.x + tileSize_, min.y + tileSize_);

			auto drawList = ImGui::GetWindowDrawList();
			drawList->AddRectFilled(min, max, tileColorMap_[data.tileData[index]]);
		}
	}

	//画面とマウスの当たり判定(巨大なInvisibleButtonがあるため)
	bool uiUsingMouse = ImGui::IsItemHovered();

	if (uiUsingMouse) {
		ImVec2 mouse = ImGui::GetMousePos();
		ImVec2 local(mouse.x - origin.x, mouse.y - origin.y);

		int tileX = int(local.x / tileSize_);
		int tileY = int(local.y / tileSize_);

		if (tileX >= 0 && tileX < data.width &&
			tileY >= 0 && tileY < data.height) {

			// ホバー表示（確認用）
			ImVec2 hMin(
				origin.x + tileX * tileSize_,
				origin.y + tileY * tileSize_
			);
			ImVec2 hMax(
				hMin.x + tileSize_,
				hMin.y + tileSize_
			);

			drawList->AddRect(
				hMin, hMax,
				IM_COL32(255, 255, 0, 255),
				0.0f, 0, 2.0f
			);

			// クリック
			if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
				if (!preLeftMouse_) {
					undoStack_.push_back(data.tileData);
					preLeftMouse_ = true;
				}
				int idx = tileY * data.width + tileX;
				data.tileData[idx] = pencil_.type;
			} else {
				preLeftMouse_ = false;
			}
			//右クリックで空気にする
			if(ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
				if(!preRightMouse_) {
					undoStack_.push_back(data.tileData);
					preRightMouse_ = true;
				}
				int idx = tileY * data.width + tileX;
				data.tileData[idx] = TileType::Air;
			} else {
				preRightMouse_ = false;
			}
		}
	}

	ImGui::EndChild();

	ImGui::End();

	ImGui::Begin("Pallet");

	drawList = ImGui::GetWindowDrawList();

	origin = ImGui::GetCursorScreenPos();
	const float baseSize = 24.0f;
	const float selectedSize = 32.0f;
	
	for (int i = 0; i < static_cast<int>(TileType::Count); ++i) {
		Vector2 pos = { float(i % 4) * 36.0f, float(i / 4) * 36.0f };

		ImVec2 min = ImVec2(origin.x + pos.x, origin.y + pos.y);
		ImVec2 max{};
		if (i == static_cast<int>(pencil_.type)) {
			max = ImVec2(origin.x + pos.x + selectedSize, origin.y + pos.y + selectedSize);
		} else {
			max = ImVec2(origin.x + pos.x + baseSize, origin.y + pos.y + baseSize);
		}

		drawList->AddRectFilled(min, max, tileColorMap_[static_cast<TileType>(i)]);
	}

	if (true) {
		ImVec2 mouse = ImGui::GetMousePos();
		ImVec2 local = { mouse.x - origin.x, mouse.y - origin.y };

		int tileX = int(local.x / 36.0f);
		int tileY = int(local.y / 36.0f);

		if (tileX >= 0 && tileX < 4 &&
			tileY >= 0 && tileY < float(TileType::Count) / 4.0f) {

			int idx = tileY * 4 + tileX;
			float tmpSize = baseSize;
			if (idx == static_cast<int>(pencil_.type)) {
				tmpSize = selectedSize;
			}

			// ホバー表示（確認用）
			ImVec2 hMin(
				origin.x + tileX * 36.0f,
				origin.y + tileY * 36.0f
			);
			ImVec2 hMax(
				hMin.x + tmpSize,
				hMin.y + tmpSize
			);

			drawList->AddRect(
				hMin, hMax,
				IM_COL32(255, 255, 0, 255),
				0.0f, 0, 2.0f
			);

			// クリック
			if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
				pencil_.type = TileType(idx);
			}
		}
	}

	mapSize = ImVec2(
		float(4 * tileSize_),
		(float(float(TileType::Count) / 4.0f + 1.0f) * 36.f)
	);

	ImGui::InvisibleButton("Pallet", mapSize);

	Vector4 buffer = ConvertColorForImGui(uint32_t(tileColorMap_[pencil_.type]));
	ImGui::ColorEdit4("EditColor", &buffer.x);
	tileColorMap_[pencil_.type] = ImU32(ConvertColorForImGui(buffer));

	ImGui::Separator();
	ImGui::DragInt("TileSize", &tileSize_, 1.0f, 8, 128);

	if (ImGui::Button("Clear")) {
		undoStack_.push_back(data.tileData);
		std::fill(data.tileData.begin(), data.tileData.end(), TileType::Air);
	}
	
	ImGui::End();

	if (Input::GetKeyState(DIK_LCONTROL) && Input::GetKeyState(DIK_Z)) {
		if (!undoStack_.empty() && !preUndo_) {
			data.tileData = undoStack_.back();
			undoStack_.pop_back();
			preUndo_ = true;
		}
	} else {
		preUndo_ = false;
	}

#endif
}

void MapEditor::Campus::Save() {
	binaryManager_.RegisterOutput(tileSize_);
	for(const auto& [type, color] : tileColorMap_) {
		binaryManager_.RegisterOutput(static_cast<uint32_t>(color));
	}
	binaryManager_.Write("EditConfigure");
}

void MapEditor::Campus::Load() {
	auto values = binaryManager_.Read("EditConfigure");

	if (values.empty()) {
		return;
	}

	int index = 0;
	tileSize_ = BinaryManager::Reverse<int>(values[index++].get());
	for (index; index < values.size(); ++index) {
		tileColorMap_[static_cast<TileType>(index - 1)] = static_cast<ImU32>(BinaryManager::Reverse<uint32_t>(values[index].get()));
	}
}
