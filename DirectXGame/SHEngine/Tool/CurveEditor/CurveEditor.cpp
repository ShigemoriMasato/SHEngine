#include "CurveEditor.h"
#include "CurveEditorLog.h"
#include "CurvePlayer.h"
#include "Data/CurveAssetIO.h"
#include <Tool/Binary/BinaryManager.h>
#include <Tool/ImGui/ImGuiWindowManager.h>

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <cstdio>
#include <filesystem>

#ifdef USE_IMGUI
#include <imgui/imgui.h>
#endif

namespace fs = std::filesystem;

namespace {
	constexpr const char* kChannelNames[CurveTrack::kMaxDimension] = { "X", "Y", "Z", "W" };
	constexpr const char* kDimensionLabels[CurveTrack::kMaxDimension] = {
		"1 (float)", "2 (float2)", "3 (float3)", "4 (float4)"
	};

#ifdef USE_IMGUI
	// CameraEditor の Position X/Y/Z と同じ色 + W
	constexpr ImU32 kChannelColors[CurveTrack::kMaxDimension] = {
		IM_COL32(235, 80, 80, 255), IM_COL32(90, 210, 100, 255),
		IM_COL32(80, 135, 245, 255), IM_COL32(230, 200, 80, 255)
	};

	float DistanceSquared(ImVec2 a, ImVec2 b) {
		const float x = a.x - b.x;
		const float y = a.y - b.y;
		return x * x + y * y;
	}

	void CopyToBuffer(char* buffer, size_t size, const std::string& text) {
		std::snprintf(buffer, size, "%s", text.c_str());
	}
#endif
}

//==============================================================
// 生成・破棄
//==============================================================

CurveEditor::CurveEditor(const std::string& categoryName)
	: categoryName_(CurveAssetIO::SanitizeName(categoryName)) {
	if (categoryName_ != categoryName) {
		CurveEditorLog::Get()->warn("Category name \"{}\" was changed to \"{}\"", categoryName, categoryName_);
	}
	LoadAll();
	LoadCaches();
	if (!items_.empty()) {
		SelectItem(0);
	}
	CurveEditorLog::Get()->info("CurveEditor [{}] initialized: {} item(s) in {}",
		categoryName_, items_.size(), CurveAssetIO::GetDirectory(categoryName_));
}

CurveEditor::~CurveEditor() {
	SetPreview(false);
	// 保存せずに閉じた場合はキャッシュへ書き出す
	int unsavedCount = 0;
	for (Item& item : items_) {
		if (item.IsDirty()) {
			++unsavedCount;
			WriteCache(item);
		}
	}

	if (unsavedCount > 0) {
		CurveEditorLog::Get()->warn("CurveEditor [{}]: {} unsaved item(s) were written to cache. They will be restored next time.",
			categoryName_, unsavedCount);
	}
}

//==============================================================
// ファイル
//==============================================================

void CurveEditor::LoadAll() {
	items_.clear();
	for (const std::string& name : CurveAssetIO::ListItems(categoryName_)) {
		CurveAsset asset;
		if (!CurveAssetIO::Load(asset, categoryName_, name)) {
			SetStatus("Failed to load: " + name);
			CurveEditorLog::Get()->error("CurveEditor [{}]: skipped item \"{}\" because it could not be loaded", categoryName_, name);
			continue;
		}
		Item item;
		item.name = name;
		item.savedName = name;
		item.asset = asset;
		item.savedAsset = asset;
		item.committed = asset;
		items_.push_back(std::move(item));
	}
}

void CurveEditor::LoadCaches() {
	std::error_code ec;
	const fs::path dir(CurveAssetIO::GetDirectory(categoryName_));
	if (!fs::is_directory(dir, ec)) {
		return;
	}

	std::vector<std::string> cacheNames;
	for (const auto& entry : fs::directory_iterator(dir, ec)) {
		if (!entry.is_regular_file(ec)) continue;
		try {
			if (entry.path().extension().string() == CurveAssetIO::kCacheExtension) {
				cacheNames.push_back(entry.path().stem().string());
			}
		} catch (const std::exception&) {}
	}

	for (const std::string& cacheName : cacheNames) {
		BinaryManager bin;
		const std::string cachePath = CurveAssetIO::GetRelativePath(categoryName_, cacheName, CurveAssetIO::kCacheExtension);
		if (!bin.Boot(cachePath)) {
			SetStatus("Failed to load cache: " + cacheName);
			CurveEditorLog::Get()->error("Failed to open cache (kept on disk): {}", cachePath);
			continue;
		}
		std::string signature, name, savedName;
		CurveAsset asset;
		if (!bin.Reverse(signature) || signature != kCacheSignature ||
			!bin.Reverse(name) || !bin.Reverse(savedName) ||
			!CurveAssetIO::Read(bin, asset)) {
			SetStatus("Failed to load cache: " + cacheName);
			CurveEditorLog::Get()->error("Invalid cache (kept on disk): {}", cachePath);
			continue;
		}

		// 保存済みファイルに対応するキャッシュならそのアイテムを上書きする
		int index = savedName.empty() ? -1 : FindItem(savedName);
		if (index >= 0 && items_[index].savedName != savedName) {
			index = -1;
		}
		if (index < 0) {
			Item item;
			items_.push_back(std::move(item));
			index = static_cast<int>(items_.size()) - 1;
		}
		Item& item = items_[index];
		item.name = MakeUniqueName(CurveAssetIO::SanitizeName(name), &item);
		item.asset = asset;
		item.committed = asset;
		item.undo.clear();
		item.redo.clear();
		item.cacheUpToDate = false; // 次の自動保存で再びキャッシュに書く

		RemoveCacheFile(cacheName);
		SetStatus("Restored unsaved changes: " + item.name);
		CurveEditorLog::Get()->info("CurveEditor [{}]: restored unsaved changes of \"{}\" from cache{}",
			categoryName_, item.name, savedName.empty() ? " (not saved yet)" : "");
	}
	std::sort(items_.begin(), items_.end(), [](const Item& a, const Item& b) { return a.name < b.name; });
}

bool CurveEditor::SaveItem(Item& item) {
	item.asset.Sort();
	if (!CurveAssetIO::Save(item.asset, categoryName_, item.name)) {
		SetStatus("Failed to save: " + item.name);
		CurveEditorLog::Get()->error("CurveEditor [{}]: failed to save \"{}\"", categoryName_, item.name);
		return false;
	}
	// 名前を変えていた場合は古いファイルを消す
	if (!item.savedName.empty() && item.savedName != item.name) {
		CurveEditorLog::Get()->info("CurveEditor [{}]: removed old file \"{}\" after rename", categoryName_, item.savedName);
		RemoveItemFile(item.savedName);
		RemoveCacheFile(item.savedName);
	}
	RemoveCacheFile(item.name);
	item.savedName = item.name;
	item.savedAsset = item.asset;
	item.cacheUpToDate = true;
	const std::string savedPath = CurveAssetIO::GetDirectory(categoryName_) + item.name + CurveAssetIO::kExtension;
	SetStatus("Saved: " + savedPath);
	CurveEditorLog::Get()->info("Saved: {}", savedPath);
	return true;
}

bool CurveEditor::WriteCache(Item& item) {
	if (!CurveAssetIO::EnsureDirectory(categoryName_)) {
		CurveEditorLog::Get()->error("CurveEditor [{}]: failed to write cache of \"{}\"", categoryName_, item.name);
		return false;
	}
	const std::string cachePath = CurveAssetIO::GetRelativePath(categoryName_, item.name, CurveAssetIO::kCacheExtension);
	BinaryManager bin;
	bin.Register(kCacheSignature);
	bin.Register(item.name);
	bin.Register(item.savedName);
	CurveAssetIO::Write(bin, item.asset);
	try {
		bin.Write(cachePath);
	} catch (const std::exception& e) {
		CurveEditorLog::Get()->error("Failed to write cache: {} ({})", cachePath, e.what());
		return false;
	}
	item.cacheUpToDate = true;
	CurveEditorLog::Get()->debug("Cache written: {}", cachePath);
	return true;
}

void CurveEditor::RemoveCacheFile(const std::string& itemName) {
	const std::string path = CurveAssetIO::GetDirectory(categoryName_) + itemName + CurveAssetIO::kCacheExtension;
	std::error_code ec;
	fs::remove(path, ec);
	if (ec) {
		CurveEditorLog::Get()->error("Failed to remove cache: {} ({})", path, ec.message());
	}
}

void CurveEditor::RemoveItemFile(const std::string& itemName) {
	const std::string path = CurveAssetIO::GetDirectory(categoryName_) + itemName + CurveAssetIO::kExtension;
	std::error_code ec;
	fs::remove(path, ec);
	if (ec) {
		CurveEditorLog::Get()->error("Failed to remove file: {} ({})", path, ec.message());
	}
}

void CurveEditor::AutoSave(float deltaTime) {
	autoSaveTimer_ += deltaTime;
	if (autoSaveTimer_ < kAutoSaveInterval) {
		return;
	}
	autoSaveTimer_ = 0.0f;

	for (Item& item : items_) {
		if (item.cacheUpToDate) continue;
		if (item.IsDirty()) {
			WriteCache(item);
		} else {
			// Undo などで保存済みの状態に戻った
			RemoveCacheFile(item.name);
			item.cacheUpToDate = true;
		}
	}
}

void CurveEditor::SetStatus(const std::string& message) {
	statusMessage_ = message;
	statusTimer_ = 4.0f;
}

bool CurveEditor::SaveSelected() {
	Item* item = GetSelectedItem();
	return item ? SaveItem(*item) : false;
}

void CurveEditor::SaveAll() {
	for (Item& item : items_) {
		if (item.IsDirty()) {
			SaveItem(item);
		}
	}
}

//==============================================================
// アイテム
//==============================================================

CurveEditor::Item* CurveEditor::GetSelectedItem() {
	if (selectedItem_ < 0 || selectedItem_ >= static_cast<int>(items_.size())) {
		return nullptr;
	}
	return &items_[selectedItem_];
}

const CurveAsset* CurveEditor::GetAsset(const std::string& itemName) const {
	const int index = FindItem(itemName);
	return index < 0 ? nullptr : &items_[index].asset;
}

int CurveEditor::FindItem(const std::string& name) const {
	for (int i = 0; i < static_cast<int>(items_.size()); ++i) {
		if (items_[i].name == name) return i;
	}
	return -1;
}

std::string CurveEditor::MakeUniqueName(const std::string& baseName, const Item* ignore) const {
	auto used = [&](const std::string& name) {
		for (const Item& item : items_) {
			if (&item != ignore && item.name == name) return true;
		}
		return false;
	};
	if (!used(baseName)) {
		return baseName;
	}
	for (int i = 1;; ++i) {
		const std::string name = baseName + "_" + std::to_string(i);
		if (!used(name)) return name;
	}
}

void CurveEditor::SelectItem(int index) {
	selectedItem_ = index;
	selectedTrack_ = 0;
	selectedChannel_ = 0;
	selectedKeyId_ = UINT32_MAX;
	dragTarget_ = DragTarget::kNone;
	timer_ = 0.0f;
	isPlaying_ = false;
	fitViewRequested_ = true;
	forceApply_ = true;
#ifdef USE_IMGUI
	if (const Item* item = GetSelectedItem()) {
		CopyToBuffer(renameBuffer_, sizeof(renameBuffer_), item->name);
	}
#endif
}

CurveTrack* CurveEditor::GetSelectedTrack(Item& item) {
	auto& tracks = item.asset.GetTracks();
	if (tracks.empty()) {
		return nullptr;
	}
	selectedTrack_ = std::clamp(selectedTrack_, 0, static_cast<int>(tracks.size()) - 1);
	CurveTrack& track = tracks[selectedTrack_];
	selectedChannel_ = std::clamp(selectedChannel_, 0, track.GetDimension() - 1);
	return &track;
}

//==============================================================
// Undo / Redo
//==============================================================

void CurveEditor::CommitIfChanged(Item& item) {
	if (item.asset == item.committed) {
		return;
	}
	item.undo.push_back(item.committed);
	if (item.undo.size() > kMaxUndo) {
		item.undo.pop_front();
	}
	item.redo.clear();
	item.committed = item.asset;
	item.cacheUpToDate = false;
}

void CurveEditor::Undo(Item& item) {
	CommitIfChanged(item);
	if (item.undo.empty()) {
		return;
	}
	item.redo.push_back(item.asset);
	item.asset = item.undo.back();
	item.undo.pop_back();
	item.committed = item.asset;
	item.cacheUpToDate = false;
	dragTarget_ = DragTarget::kNone;
	if (CurveTrack* track = GetSelectedTrack(item)) {
		if (!track->GetChannel(selectedChannel_).FindKey(selectedKeyId_)) selectedKeyId_ = UINT32_MAX;
	}
}

void CurveEditor::Redo(Item& item) {
	CommitIfChanged(item);
	if (item.redo.empty()) {
		return;
	}
	item.undo.push_back(item.asset);
	item.asset = item.redo.back();
	item.redo.pop_back();
	item.committed = item.asset;
	item.cacheUpToDate = false;
	dragTarget_ = DragTarget::kNone;
	if (CurveTrack* track = GetSelectedTrack(item)) {
		if (!track->GetChannel(selectedChannel_).FindKey(selectedKeyId_)) selectedKeyId_ = UINT32_MAX;
	}
}

//==============================================================
// プレビュー
//==============================================================

void CurveEditor::SetBinder(CurveBinder* binder) {
	if (binder_ == binder) {
		return;
	}
	const bool wasPreview = isPreview_;
	SetPreview(false);
	if (binder_ && binder) {
		CurveEditorLog::Get()->warn("CurveEditor [{}]: binder was replaced (only one binder can be set)", categoryName_);
	}
	binder_ = binder;
	CurveEditorLog::Get()->debug("CurveEditor [{}]: binder {} ({} binding(s))", categoryName_,
		binder ? "set" : "cleared", binder ? binder->GetBindingNames().size() : 0);
	if (wasPreview) {
		SetPreview(true);
	}
}

void CurveEditor::SetPreview(bool enable) {
	if (enable == isPreview_) {
		return;
	}
	if (enable) {
		if (!binder_) {
			CurveEditorLog::Get()->warn("CurveEditor [{}]: preview needs a binder. Call SetBinder first.", categoryName_);
			return;
		}
		binder_->Capture();
		forceApply_ = true;
	} else if (binder_) {
		binder_->Restore();
	}
	isPreview_ = enable;
	CurveEditorLog::Get()->debug("CurveEditor [{}]: preview {}", categoryName_, enable ? "on" : "off (values restored)");
}

void CurveEditor::UpdatePlayback(float deltaTime) {
	Item* item = GetSelectedItem();
	if (!item) {
		isPlaying_ = false;
		return;
	}
	if (isPlaying_) {
		if (CurvePlayer::Advance(timer_, deltaTime * playSpeed_, item->asset.duration, item->asset.loop)) {
			isPlaying_ = false;
		}
	}

	if (isPreview_ && binder_) {
		const bool changed = forceApply_ || isPlaying_ || lastAppliedItem_ != selectedItem_ ||
			lastAppliedTime_ != timer_ || !(lastAppliedAsset_ == item->asset);
		if (changed) {
			binder_->Apply(item->asset, timer_);
			lastAppliedItem_ = selectedItem_;
			lastAppliedTime_ = timer_;
			lastAppliedAsset_ = item->asset;
			forceApply_ = false;
		}
	}
}

void CurveEditor::AddKeyAtTimer(Item& item) {
	CurveTrack* track = GetSelectedTrack(item);
	if (!track) {
		return;
	}
	// バインド先があればその現在値、無ければカーブの現在値でキーを打つ(CameraEditor の Add Key と同じ)
	CurveBinder::Values values = {};
	if (!binder_ || !binder_->GetCurrentValue(track->GetName(), values)) {
		track->Evaluate(timer_, values.data());
	}
	track->AddKeyAll(timer_, values.data());
	item.asset.Sort();
	fitViewRequested_ = true;
}

//==============================================================
// 更新
//==============================================================

void CurveEditor::Update(float deltaTime) {
#ifdef USE_IMGUI
	if (statusTimer_ > 0.0f) {
		statusTimer_ -= deltaTime;
	}

	// ウィンドウを閉じた状態でマウスを離した場合もドラッグを終了させる
	if (dragTarget_ != DragTarget::kNone && !ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
		if (Item* item = GetSelectedItem()) {
			item->asset.Sort();
		}
		dragTarget_ = DragTarget::kNone;
	}

	// 操作が一段落したら Undo 履歴に積む
	if (Item* item = GetSelectedItem()) {
		if (!ImGui::IsAnyItemActive() && dragTarget_ == DragTarget::kNone) {
			CommitIfChanged(*item);
		}
	}
#endif

	UpdatePlayback(deltaTime);
	AutoSave(deltaTime);
}

void CurveEditor::DrawWindow() {
#ifdef USE_IMGUI
	const std::string title = "Curve Editor [" + categoryName_ + "]###CurveEditor_" + categoryName_;
	if (ImGui::Begin(title.c_str())) {
		DrawImGui();
	}
	ImGui::End();
#endif
}

#ifdef USE_IMGUI

void CurveEditor::DrawImGui() {
	{
		Item* item = GetSelectedItem();

		// ---- ツールバー ----
		ImGui::BeginDisabled(!item);
		if (ImGui::Button("Save (Ctrl+S)")) SaveSelected();
		ImGui::EndDisabled();
		ImGui::SameLine();
		if (ImGui::Button("Save All")) SaveAll();
		ImGui::SameLine();
		ImGui::BeginDisabled(!item || (item->undo.empty() && item->asset == item->committed));
		if (ImGui::Button("Undo") && item) Undo(*item);
		ImGui::EndDisabled();
		ImGui::SameLine();
		ImGui::BeginDisabled(!item || item->redo.empty());
		if (ImGui::Button("Redo") && item) Redo(*item);
		ImGui::EndDisabled();
		if (statusTimer_ > 0.0f && !statusMessage_.empty()) {
			ImGui::SameLine();
			ImGui::TextDisabled("%s", statusMessage_.c_str());
		}

		// ---- 左: アイテム一覧 ----
		ImGui::BeginChild("##CurveItems", ImVec2(220.0f, 0.0f), ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX);
		DrawItemList();
		ImGui::EndChild();
		ImGui::SameLine();

		// ---- 右: 選択中アイテム ----
		ImGui::BeginChild("##CurveDetail", ImVec2(0.0f, 0.0f), ImGuiChildFlags_Borders);
		item = GetSelectedItem();
		if (item) {
			DrawItemDetail(*item);
		} else {
			ImGui::TextDisabled("No item selected. Create one on the left.");
		}
		ImGui::EndChild();

		if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)) {
			HandleShortcuts(GetSelectedItem());
		}
	}
}

void CurveEditor::HandleShortcuts(Item* item) {
	if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_S)) {
		SaveSelected();
	}
	if (!item || ImGui::GetIO().WantTextInput) {
		return;
	}
	if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_Z)) {
		Undo(*item);
	}
	if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_Y) ||
		ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiMod_Shift | ImGuiKey_Z)) {
		Redo(*item);
	}
	if (ImGui::IsKeyChordPressed(ImGuiKey_I)) {
		AddKeyAtTimer(*item);
	}
	if (ImGui::IsKeyChordPressed(ImGuiKey_Space)) {
		isPlaying_ = !isPlaying_;
	}
}

void CurveEditor::DrawItemList() {
	ImGui::SeparatorText("Items");
	int clicked = -1;
	for (int i = 0; i < static_cast<int>(items_.size()); ++i) {
		const Item& item = items_[i];
		const std::string label = item.name + (item.IsDirty() ? " *" : "") + "##CurveItem" + std::to_string(i);
		if (ImGui::Selectable(label.c_str(), selectedItem_ == i)) {
			clicked = i;
		}
	}
	if (clicked >= 0 && clicked != selectedItem_) {
		SelectItem(clicked);
	}

	ImGui::Separator();
	ImGui::SetNextItemWidth(-60.0f);
	ImGui::InputText("##NewItemName", newItemName_, sizeof(newItemName_));
	ImGui::SameLine();
	if (ImGui::Button("New")) {
		Item item;
		item.name = MakeUniqueName(CurveAssetIO::SanitizeName(newItemName_));
		item.cacheUpToDate = false;
		CurveEditorLog::Get()->info("CurveEditor [{}]: created item \"{}\"", categoryName_, item.name);
		items_.push_back(std::move(item));
		SelectItem(static_cast<int>(items_.size()) - 1);
	}

	Item* selected = GetSelectedItem();
	if (!selected) {
		return;
	}

	ImGui::SetNextItemWidth(-60.0f);
	ImGui::InputText("##RenameItem", renameBuffer_, sizeof(renameBuffer_));
	ImGui::SameLine();
	if (ImGui::Button("Rename")) {
		const std::string newName = MakeUniqueName(CurveAssetIO::SanitizeName(renameBuffer_), selected);
		if (newName != selected->name) {
			CurveEditorLog::Get()->info("CurveEditor [{}]: renamed item \"{}\" -> \"{}\" (file is renamed on save)",
				categoryName_, selected->name, newName);
			RemoveCacheFile(selected->name);
			selected->name = newName;
			selected->cacheUpToDate = false;
		}
		CopyToBuffer(renameBuffer_, sizeof(renameBuffer_), selected->name);
	}

	if (ImGui::Button("Duplicate")) {
		Item copy;
		copy.name = MakeUniqueName(selected->name + "_Copy");
		copy.asset = selected->asset;
		copy.committed = selected->asset;
		copy.cacheUpToDate = false;
		CurveEditorLog::Get()->info("CurveEditor [{}]: duplicated item \"{}\" -> \"{}\"", categoryName_, selected->name, copy.name);
		items_.push_back(std::move(copy));
		SelectItem(static_cast<int>(items_.size()) - 1);
		selected = GetSelectedItem();
	}
	ImGui::SameLine();
	if (ImGui::Button("Delete")) {
		ImGui::OpenPopup("Delete Curve Item");
	}
	if (ImGui::BeginPopupModal("Delete Curve Item", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::Text("Delete \"%s\"?", selected->name.c_str());
		ImGui::TextDisabled("The saved file will also be removed.");
		if (ImGui::Button("Delete", ImVec2(100.0f, 0.0f))) {
			if (!selected->savedName.empty()) {
				RemoveItemFile(selected->savedName);
				RemoveCacheFile(selected->savedName);
			}
			RemoveCacheFile(selected->name);
			CurveEditorLog::Get()->info("CurveEditor [{}]: deleted item \"{}\"{}", categoryName_, selected->name,
				selected->savedName.empty() ? "" : " and its file");
			items_.erase(items_.begin() + selectedItem_);
			SelectItem(items_.empty() ? -1 : std::min(selectedItem_, static_cast<int>(items_.size()) - 1));
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(100.0f, 0.0f))) {
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

void CurveEditor::DrawItemDetail(Item& item) {
	ImGui::Text("%s%s", item.name.c_str(), item.IsDirty() ? " *" : "");
	ImGui::SameLine();
	ImGui::TextDisabled("(%s)", (CurveAssetIO::GetDirectory(categoryName_) + item.name + CurveAssetIO::kExtension).c_str());

	DrawPlayback(item);
	DrawTrackList(item);
	DrawKeyInspector(item);
	DrawCurveCanvas(item);
}

void CurveEditor::DrawPlayback(Item& item) {
	ImGui::SeparatorText("Playback");

	bool preview = isPreview_;
	ImGui::BeginDisabled(!binder_);
	if (ImGui::Checkbox("Preview", &preview)) {
		SetPreview(preview);
	}
	ImGui::EndDisabled();
	if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
		ImGui::SetTooltip(binder_ ? "Write values to the bound targets. Turning it off restores the original values."
			: "No binder. Call CurveEditor::SetBinder().");
	}
	ImGui::SameLine();
	if (ImGui::Button(isPlaying_ ? "Pause" : "Play")) {
		isPlaying_ = !isPlaying_;
	}
	ImGui::SameLine();
	if (ImGui::Button("Stop")) {
		isPlaying_ = false;
		timer_ = 0.0f;
	}
	ImGui::SameLine();
	if (ImGui::Button("Add Key (I)")) {
		AddKeyAtTimer(item);
	}
	ImGui::SameLine();
	ImGui::Checkbox("Loop", &item.asset.loop);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(80.0f);
	ImGui::DragFloat("Speed", &playSpeed_, 0.01f, 0.0f, 10.0f, "%.2fx");

	// CameraEditor と同じ粒度
	float duration = std::max(item.asset.duration, 0.01f);
	ImGui::SetNextItemWidth(120.0f);
	if (ImGui::DragFloat("Duration", &duration, 0.01f, 0.01f, 3600.0f, "%.2f s")) {
		item.asset.duration = duration;
		timer_ = std::min(timer_, item.asset.duration);
	}
	ImGui::SameLine();
	ImGui::SetNextItemWidth(-1.0f);
	ImGui::SliderFloat("##CurveTime", &timer_, 0.0f, duration, "%.3f s");
}

void CurveEditor::DrawTrackList(Item& item) {
	ImGui::SeparatorText("Tracks");
	auto& tracks = item.asset.GetTracks();

	if (ImGui::BeginTable("##CurveTracks", 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV |
		ImGuiTableFlags_ScrollY, ImVec2(0.0f, std::min(24.0f * (tracks.size() + 1), 130.0f)))) {
		ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 80.0f);
		ImGui::TableSetupColumn("Binding", ImGuiTableColumnFlags_WidthFixed, 140.0f);
		for (int i = 0; i < static_cast<int>(tracks.size()); ++i) {
			const CurveTrack& track = tracks[i];
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::PushID(i);
			if (ImGui::Selectable(track.GetName().c_str(), selectedTrack_ == i, ImGuiSelectableFlags_SpanAllColumns)) {
				if (selectedTrack_ != i) {
					selectedTrack_ = i;
					selectedChannel_ = 0;
					selectedKeyId_ = UINT32_MAX;
					fitViewRequested_ = true;
				}
			}
			ImGui::PopID();
			ImGui::TableNextColumn();
			ImGui::TextUnformatted(kDimensionLabels[track.GetDimension() - 1]);
			ImGui::TableNextColumn();
			const int boundDimension = binder_ ? binder_->GetDimension(track.GetName()) : 0;
			if (boundDimension == 0) {
				ImGui::TextDisabled("unbound");
			} else if (boundDimension != track.GetDimension()) {
				ImGui::TextColored(ImVec4(1.0f, 0.75f, 0.3f, 1.0f), "bound (dim %d)", boundDimension);
			} else {
				ImGui::TextColored(ImVec4(0.45f, 0.9f, 0.5f, 1.0f), "bound");
			}
		}
		ImGui::EndTable();
	}

	// 追加
	ImGui::SetNextItemWidth(150.0f);
	ImGui::InputText("##NewTrackName", newTrackName_, sizeof(newTrackName_));
	ImGui::SameLine();
	ImGui::SetNextItemWidth(110.0f);
	int dimensionIndex = newTrackDimension_ - 1;
	if (ImGui::Combo("##NewTrackDim", &dimensionIndex, kDimensionLabels, CurveTrack::kMaxDimension)) {
		newTrackDimension_ = dimensionIndex + 1;
	}
	ImGui::SameLine();
	if (ImGui::Button("Add Track")) {
		if (item.asset.AddTrack(newTrackName_, newTrackDimension_)) {
			CurveEditorLog::Get()->debug("CurveEditor [{}]: added track \"{}\" (dimension {})", categoryName_, newTrackName_, newTrackDimension_);
			if (binder_) {
				const int bindDimension = binder_->GetDimension(newTrackName_);
				if (bindDimension == 0) {
					CurveEditorLog::Get()->warn("CurveEditor [{}]: track \"{}\" has no binding with the same name. It is not previewed.",
						categoryName_, newTrackName_);
				} else if (bindDimension != newTrackDimension_) {
					CurveEditorLog::Get()->warn("CurveEditor [{}]: track \"{}\" dimension {} differs from the binding ({}). Only the common components are applied.",
						categoryName_, newTrackName_, newTrackDimension_, bindDimension);
				}
			}
			selectedTrack_ = static_cast<int>(tracks.size()) - 1;
			selectedChannel_ = 0;
			selectedKeyId_ = UINT32_MAX;
			fitViewRequested_ = true;
		} else {
			SetStatus("Track name is empty or already exists");
		}
	}
	if (binder_) {
		ImGui::SameLine();
		ImGui::SetNextItemWidth(130.0f);
		if (ImGui::BeginCombo("##FromBinder", "From Binder")) {
			for (const std::string& name : binder_->GetBindingNames()) {
				if (item.asset.FindTrack(name)) continue;
				if (ImGui::Selectable(name.c_str())) {
					CopyToBuffer(newTrackName_, sizeof(newTrackName_), name);
					newTrackDimension_ = binder_->GetDimension(name);
				}
			}
			ImGui::EndCombo();
		}
	}
	ImGui::SameLine();
	ImGui::BeginDisabled(tracks.empty());
	if (ImGui::Button("Remove Track") && !tracks.empty()) {
		ImGui::OpenPopup("Remove Curve Track");
	}
	ImGui::EndDisabled();
	if (ImGui::BeginPopup("Remove Curve Track")) {
		if (CurveTrack* track = GetSelectedTrack(item)) {
			ImGui::Text("Remove \"%s\"?", track->GetName().c_str());
			if (ImGui::Button("Remove")) {
				CurveEditorLog::Get()->debug("CurveEditor [{}]: removed track \"{}\"", categoryName_, track->GetName());
				item.asset.RemoveTrack(track->GetName());
				selectedTrack_ = std::max(0, selectedTrack_ - 1);
				selectedKeyId_ = UINT32_MAX;
				fitViewRequested_ = true;
				ImGui::CloseCurrentPopup();
			}
		}
		ImGui::EndPopup();
	}
}

void CurveEditor::DrawKeyInspector(Item& item) {
	CurveTrack* track = GetSelectedTrack(item);
	if (!track) {
		return;
	}
	CurveChannel& channel = track->GetChannel(selectedChannel_);
	CurveKey* key = channel.FindKey(selectedKeyId_);

	ImGui::SeparatorText("Key");
	if (!key) {
		ImGui::TextDisabled("No key selected");
		return;
	}
	ImGui::PushStyleColor(ImGuiCol_Text, kChannelColors[selectedChannel_]);
	ImGui::Text("%s.%s", track->GetName().c_str(), kChannelNames[selectedChannel_]);
	ImGui::PopStyleColor();
	ImGui::SameLine();

	ImGui::SetNextItemWidth(100.0f);
	if (ImGui::DragFloat("Time", &key->time, 0.01f, 0.0f, 3600.0f, "%.3f")) {
		key->time = std::max(0.0f, key->time);
		channel.SortKeys();
		key = channel.FindKey(selectedKeyId_);
		item.asset.Sort();
	}
	ImGui::SameLine();
	ImGui::SetNextItemWidth(100.0f);
	ImGui::DragFloat("Value", &key->value, 0.01f, 0.0f, 0.0f, "%.3f");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(130.0f);
	if (ImGui::DragFloat2("In", &key->leftHandle.x, 0.01f, 0.0f, 0.0f, "%.3f")) {
		key->leftHandle.x = std::min(0.0f, key->leftHandle.x);
	}
	ImGui::SameLine();
	ImGui::SetNextItemWidth(130.0f);
	if (ImGui::DragFloat2("Out", &key->rightHandle.x, 0.01f, 0.0f, 0.0f, "%.3f")) {
		key->rightHandle.x = std::max(0.0f, key->rightHandle.x);
	}
	ImGui::SameLine();
	if (ImGui::Button("Delete Key")) {
		channel.RemoveKey(selectedKeyId_);
		selectedKeyId_ = UINT32_MAX;
	}
}

// CameraEditor::CurveDraw を Track の次元数に合わせて一般化したもの
void CurveEditor::DrawCurveCanvas(Item& item) {
	CurveTrack* track = GetSelectedTrack(item);
	ImGui::SeparatorText("Curves");
	if (!track) {
		ImGui::TextDisabled("Add a track to edit curves.");
		return;
	}
	const int dimension = track->GetDimension();

	for (int i = 0; i < dimension; ++i) {
		ImGui::PushID(i);
		ImGui::PushStyleColor(ImGuiCol_Text, kChannelColors[i]);
		if (ImGui::RadioButton("##Target", selectedChannel_ == i)) {
			if (selectedChannel_ != i) selectedKeyId_ = UINT32_MAX;
			selectedChannel_ = i;
		}
		if (ImGui::IsItemHovered()) ImGui::SetTooltip("Double-click adds keys to this channel");
		ImGui::SameLine(0.0f, 2.0f);
		ImGui::Checkbox(kChannelNames[i], &curveVisible_[i]);
		ImGui::PopStyleColor();
		ImGui::SameLine();
		ImGui::PopID();
	}
	if (ImGui::Button("Frame All")) fitViewRequested_ = true;
	ImGui::SameLine();
	ImGui::SetNextItemWidth(115.0f);
	float valuePerGrid = valuePerGrid_;
	if (ImGui::DragFloat("Value / Grid", &valuePerGrid, std::max(valuePerGrid_ * 0.01f, 0.001f),
		0.001f, 1000000.0f, "%.3f")) {
		valuePerGrid_ = std::max(valuePerGrid, 0.001f);
		const float center = (viewValueMin_ + viewValueMax_) * 0.5f;
		const float halfRange = valuePerGrid_ * 5.0f;
		viewValueMin_ = center - halfRange;
		viewValueMax_ = center + halfRange;
	}
	ImGui::TextDisabled("LMB: drag  Double-click: add  Wheel: zoom  RMB: pan  Delete: remove  I: key  Space: play");

	const ImVec2 canvasSize(std::max(ImGui::GetContentRegionAvail().x, 300.0f),
		std::max(ImGui::GetContentRegionAvail().y, 280.0f));
	const ImVec2 canvasMin = ImGui::GetCursorScreenPos();
	const ImVec2 canvasMax(canvasMin.x + canvasSize.x, canvasMin.y + canvasSize.y);
	ImGui::InvisibleButton("##CurveCanvas", canvasSize,
		ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
	const bool hovered = ImGui::IsItemHovered();
	const bool active = ImGui::IsItemActive();
	ImDrawList* drawList = ImGui::GetWindowDrawList();
	drawList->AddRectFilled(canvasMin, canvasMax, IM_COL32(24, 26, 31, 255));
	drawList->PushClipRect(canvasMin, canvasMax, true);

	if (fitViewRequested_) {
		float maxTime = std::max(1.0f, item.asset.duration);
		float minValue = FLT_MAX;
		float maxValue = -FLT_MAX;
		for (int c = 0; c < dimension; ++c) {
			if (!curveVisible_[c]) continue;
			for (const CurveKey& key : track->GetChannel(c).GetKeys()) {
				minValue = std::min(minValue, std::min(key.value, key.value + key.leftHandle.y));
				maxValue = std::max(maxValue, std::max(key.value, key.value + key.rightHandle.y));
				maxTime = std::max(maxTime, key.time);
			}
		}
		if (minValue == FLT_MAX) {
			minValue = -1.0f;
			maxValue = 1.0f;
		}
		const float padding = std::max((maxValue - minValue) * 0.15f, 0.5f);
		viewTimeMin_ = 0.0f;
		viewTimeMax_ = maxTime + std::max(maxTime * 0.05f, 0.1f);
		viewValueMin_ = minValue - padding;
		viewValueMax_ = maxValue + padding;
		valuePerGrid_ = (viewValueMax_ - viewValueMin_) / 10.0f;
		fitViewRequested_ = false;
	}

	auto toScreen = [&](float time, float value) {
		const float x = (time - viewTimeMin_) / (viewTimeMax_ - viewTimeMin_);
		const float y = (value - viewValueMin_) / (viewValueMax_ - viewValueMin_);
		return ImVec2(canvasMin.x + x * canvasSize.x, canvasMax.y - y * canvasSize.y);
	};
	auto toCurve = [&](ImVec2 screen) {
		const float x = (screen.x - canvasMin.x) / canvasSize.x;
		const float y = (canvasMax.y - screen.y) / canvasSize.y;
		return Vector2{
			viewTimeMin_ + x * (viewTimeMax_ - viewTimeMin_),
			viewValueMin_ + y * (viewValueMax_ - viewValueMin_)
		};
	};

	if (hovered && ImGui::GetIO().MouseWheel != 0.0f) {
		const Vector2 focus = toCurve(ImGui::GetIO().MousePos);
		const float factor = std::pow(0.85f, ImGui::GetIO().MouseWheel);
		viewTimeMin_ = focus.x + (viewTimeMin_ - focus.x) * factor;
		viewTimeMax_ = focus.x + (viewTimeMax_ - focus.x) * factor;
		viewValueMin_ = focus.y + (viewValueMin_ - focus.y) * factor;
		viewValueMax_ = focus.y + (viewValueMax_ - focus.y) * factor;
		valuePerGrid_ = (viewValueMax_ - viewValueMin_) / 10.0f;
	}
	if (active && ImGui::IsMouseDragging(ImGuiMouseButton_Right)) {
		const ImVec2 delta = ImGui::GetIO().MouseDelta;
		const float dt = -delta.x / canvasSize.x * (viewTimeMax_ - viewTimeMin_);
		const float dv = delta.y / canvasSize.y * (viewValueMax_ - viewValueMin_);
		viewTimeMin_ += dt; viewTimeMax_ += dt;
		viewValueMin_ += dv; viewValueMax_ += dv;
	}

	for (int i = 0; i <= 10; ++i) {
		const float ratio = i / 10.0f;
		const float x = canvasMin.x + canvasSize.x * ratio;
		const float y = canvasMin.y + canvasSize.y * ratio;
		drawList->AddLine(ImVec2(x, canvasMin.y), ImVec2(x, canvasMax.y), IM_COL32(55, 58, 66, 150));
		drawList->AddLine(ImVec2(canvasMin.x, y), ImVec2(canvasMax.x, y), IM_COL32(55, 58, 66, 150));
		char label[32];
		sprintf_s(label, "%.2f", viewTimeMin_ + ratio * (viewTimeMax_ - viewTimeMin_));
		drawList->AddText(ImVec2(x + 3.0f, canvasMax.y - 18.0f), IM_COL32(170, 175, 185, 255), label);
		sprintf_s(label, "%.2f", viewValueMax_ - ratio * (viewValueMax_ - viewValueMin_));
		drawList->AddText(ImVec2(canvasMin.x + 4.0f, y + 2.0f), IM_COL32(170, 175, 185, 255), label);
	}
	if (viewValueMin_ <= 0.0f && viewValueMax_ >= 0.0f) {
		const float y = toScreen(0.0f, 0.0f).y;
		drawList->AddLine(ImVec2(canvasMin.x, y), ImVec2(canvasMax.x, y), IM_COL32(120, 125, 135, 220), 1.5f);
	}
	// 終端
	{
		const float x = toScreen(item.asset.duration, 0.0f).x;
		drawList->AddLine(ImVec2(x, canvasMin.y), ImVec2(x, canvasMax.y), IM_COL32(200, 120, 70, 160), 1.0f);
	}

	float nearestDistance = 100.0f;
	int hoveredChannel = -1;
	uint32_t hoveredKey = UINT32_MAX;
	for (int c = 0; c < dimension; ++c) {
		if (!curveVisible_[c]) continue;
		const CurveChannel& channel = track->GetChannel(c);
		const auto& keys = channel.GetKeys();
		for (size_t segment = 0; segment + 1 < keys.size(); ++segment) {
			const CurveKey& a = keys[segment];
			const CurveKey& b = keys[segment + 1];
			ImVec2 previous = toScreen(a.time, a.value);
			for (int sample = 1; sample <= 48; ++sample) {
				const float time = a.time + (b.time - a.time) * (sample / 48.0f);
				const ImVec2 current = toScreen(time, channel.Evaluate(time));
				drawList->AddLine(previous, current, kChannelColors[c], 2.0f);
				previous = current;
			}
		}
		for (const CurveKey& key : keys) {
			const ImVec2 point = toScreen(key.time, key.value);
			const bool selected = selectedChannel_ == c && selectedKeyId_ == key.id;
			drawList->AddCircleFilled(point, selected ? 6.0f : 4.0f,
				selected ? IM_COL32_WHITE : kChannelColors[c]);
			const float distance = DistanceSquared(point, ImGui::GetIO().MousePos);
			if (hovered && distance < nearestDistance) {
				nearestDistance = distance;
				hoveredChannel = c;
				hoveredKey = key.id;
			}
		}
	}

	CurveChannel& selectedChannel = track->GetChannel(selectedChannel_);
	CurveKey* selectedKey = selectedChannel.FindKey(selectedKeyId_);
	int hoveredHandle = 0;
	if (selectedKey) {
		const ImVec2 keyPoint = toScreen(selectedKey->time, selectedKey->value);
		const ImVec2 leftPoint = toScreen(selectedKey->time + selectedKey->leftHandle.x,
			selectedKey->value + selectedKey->leftHandle.y);
		const ImVec2 rightPoint = toScreen(selectedKey->time + selectedKey->rightHandle.x,
			selectedKey->value + selectedKey->rightHandle.y);
		drawList->AddLine(keyPoint, leftPoint, IM_COL32(190, 190, 195, 255));
		drawList->AddLine(keyPoint, rightPoint, IM_COL32(190, 190, 195, 255));
		drawList->AddCircleFilled(leftPoint, 4.5f, IM_COL32(235, 235, 240, 255));
		drawList->AddCircleFilled(rightPoint, 4.5f, IM_COL32(235, 235, 240, 255));
		if (hovered && DistanceSquared(leftPoint, ImGui::GetIO().MousePos) < 81.0f) hoveredHandle = -1;
		if (hovered && DistanceSquared(rightPoint, ImGui::GetIO().MousePos) < 81.0f) hoveredHandle = 1;
	}

	if (hovered && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
		const Vector2 point = toCurve(ImGui::GetIO().MousePos);
		CurveKey key;
		key.time = std::max(0.0f, point.x);
		key.value = point.y;
		const float length = std::max((viewTimeMax_ - viewTimeMin_) * 0.04f, 0.02f);
		key.leftHandle = { -length, 0.0f };
		key.rightHandle = { length, 0.0f };
		selectedKeyId_ = selectedChannel.AddKey(key);
		selectedKey = nullptr;
		dragTarget_ = DragTarget::kNone;
		item.asset.duration = std::max(item.asset.duration, key.time);
	} else if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
		if (hoveredHandle != 0 && selectedKey) {
			dragTarget_ = hoveredHandle < 0 ? DragTarget::kLeftHandle : DragTarget::kRightHandle;
		} else if (hoveredKey != UINT32_MAX) {
			selectedChannel_ = hoveredChannel;
			selectedKeyId_ = hoveredKey;
			selectedKey = track->GetChannel(selectedChannel_).FindKey(selectedKeyId_);
			dragTarget_ = DragTarget::kKey;
		} else {
			selectedKeyId_ = UINT32_MAX;
			selectedKey = nullptr;
			dragTarget_ = DragTarget::kNone;
		}
	}

	if (ImGui::IsMouseDragging(ImGuiMouseButton_Left) && selectedKey) {
		const Vector2 cursor = toCurve(ImGui::GetIO().MousePos);
		if (dragTarget_ == DragTarget::kKey) {
			selectedKey->time = std::max(0.0f, cursor.x);
			selectedKey->value = cursor.y;
		} else if (dragTarget_ == DragTarget::kLeftHandle) {
			selectedKey->leftHandle = {
				std::min(0.0f, cursor.x - selectedKey->time), cursor.y - selectedKey->value
			};
		} else if (dragTarget_ == DragTarget::kRightHandle) {
			selectedKey->rightHandle = {
				std::max(0.0f, cursor.x - selectedKey->time), cursor.y - selectedKey->value
			};
		}
	}
	if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) && dragTarget_ != DragTarget::kNone) {
		track->GetChannel(selectedChannel_).SortKeys();
		item.asset.Sort();
		dragTarget_ = DragTarget::kNone;
	}
	if (selectedKeyId_ != UINT32_MAX && !ImGui::GetIO().WantTextInput &&
		ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && ImGui::IsKeyPressed(ImGuiKey_Delete)) {
		track->GetChannel(selectedChannel_).RemoveKey(selectedKeyId_);
		selectedKeyId_ = UINT32_MAX;
		item.asset.Sort();
	}

	drawList->AddLine(toScreen(timer_, viewValueMin_), toScreen(timer_, viewValueMax_),
		IM_COL32(255, 220, 75, 255), 2.0f);
	drawList->PopClipRect();
}

#else

void CurveEditor::DrawImGui() {}
void CurveEditor::HandleShortcuts(Item*) {}
void CurveEditor::DrawItemList() {}
void CurveEditor::DrawItemDetail(Item&) {}
void CurveEditor::DrawTrackList(Item&) {}
void CurveEditor::DrawPlayback(Item&) {}
void CurveEditor::DrawKeyInspector(Item&) {}
void CurveEditor::DrawCurveCanvas(Item&) {}

#endif
