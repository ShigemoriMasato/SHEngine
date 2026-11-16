#pragma once
#include "Data/CurveAsset.h"
#include "Binding/CurveBinder.h"
#include <deque>
#include <string>
#include <vector>

class ImGuiWindowManager;

/// @brief ImGui で CurveAsset を編集するエディタ。
/// Assets/Binary/CurveEditor/<カテゴリー名>/ の .bin をアイテムとして一覧表示・編集・保存する。
///
/// 自動保存: 未保存の変更は一定間隔とデストラクタでキャッシュ(<アイテム名>.cache)へ書き出す。
/// 起動時にキャッシュがあれば読み込んで(未保存状態として)復元し、キャッシュファイルは削除する。
class CurveEditor {
public:

	explicit CurveEditor(const std::string& categoryName);
	~CurveEditor();

	CurveEditor(const CurveEditor&) = delete;
	CurveEditor& operator=(const CurveEditor&) = delete;

	/// @brief プレビュー再生・Undo 確定・自動保存。ウィンドウの開閉に関係なく毎フレーム呼ぶ
	void Update(float deltaTime);

	/// @brief ウィンドウの中身を描画する(Begin/End は呼ばない。ImGuiWindowManager 用)
	void DrawImGui();
	/// @brief ImGuiWindowManager を使わずに単独ウィンドウとして描画する
	void DrawWindow();

	/// @brief プレビューの書き込み先(nullptr で解除)
	void SetBinder(CurveBinder* binder);

	const std::string& GetCategoryName() const { return categoryName_; }
	/// @brief 編集中のアセットを取得する(無ければ nullptr)
	const CurveAsset* GetAsset(const std::string& itemName) const;

	/// @brief 選択中のアイテムを保存する
	bool SaveSelected();
	/// @brief 変更のあるアイテムをすべて保存する
	void SaveAll();

private:

	struct Item {
		std::string name;
		/// @brief ディスク上のファイル名(未保存なら空)
		std::string savedName;
		CurveAsset asset;
		/// @brief 最後に保存した内容(変更判定用)
		CurveAsset savedAsset;
		/// @brief Undo の基準となる最後に確定した内容
		CurveAsset committed;
		std::deque<CurveAsset> undo;
		std::deque<CurveAsset> redo;
		/// @brief 最後にキャッシュへ書いた内容と一致しているか
		bool cacheUpToDate = true;

		bool IsDirty() const { return name != savedName || !(asset == savedAsset); }
	};

	enum class DragTarget {
		kNone,
		kKey,
		kLeftHandle,
		kRightHandle,
	};

	// ---- ファイル ----
	void LoadAll();
	void LoadCaches();
	bool SaveItem(Item& item);
	bool WriteCache(Item& item);
	void RemoveCacheFile(const std::string& itemName);
	void RemoveItemFile(const std::string& itemName);
	void AutoSave(float deltaTime);
	void SetStatus(const std::string& message);

	// ---- アイテム ----
	Item* GetSelectedItem();
	std::string MakeUniqueName(const std::string& baseName, const Item* ignore = nullptr) const;
	int FindItem(const std::string& name) const;
	void SelectItem(int index);

	// ---- Undo / Redo ----
	void CommitIfChanged(Item& item);
	void Undo(Item& item);
	void Redo(Item& item);

	// ---- プレビュー ----
	void SetPreview(bool enable);
	void UpdatePlayback(float deltaTime);

	// ---- ImGui ----
	void DrawItemList();
	void DrawItemDetail(Item& item);
	void DrawTrackList(Item& item);
	void DrawPlayback(Item& item);
	void DrawKeyInspector(Item& item);
	void DrawCurveCanvas(Item& item);
	void HandleShortcuts(Item* item);
	void AddKeyAtTimer(Item& item);

	CurveTrack* GetSelectedTrack(Item& item);

	std::string categoryName_;
	std::vector<Item> items_;
	int selectedItem_ = -1;
	int selectedTrack_ = 0;

	CurveBinder* binder_ = nullptr;
	bool isPreview_ = false;
	// プレビュー中は時間か内容が変わったときだけ書き込む(止めている間は対象を自由に動かしてキーを打てる)
	bool forceApply_ = false;
	int lastAppliedItem_ = -1;
	float lastAppliedTime_ = 0.0f;
	CurveAsset lastAppliedAsset_;

	// 再生
	bool isPlaying_ = false;
	float timer_ = 0.0f;
	float playSpeed_ = 1.0f;

	// カーブ表示(CameraEditor と同じ)
	float viewTimeMin_ = 0.0f;
	float viewTimeMax_ = 5.0f;
	float viewValueMin_ = -5.0f;
	float viewValueMax_ = 5.0f;
	float valuePerGrid_ = 1.0f;
	bool curveVisible_[CurveTrack::kMaxDimension] = { true, true, true, true };
	bool fitViewRequested_ = true;
	int selectedChannel_ = 0;
	uint32_t selectedKeyId_ = UINT32_MAX;
	DragTarget dragTarget_ = DragTarget::kNone;

	// UI 入力
	char newItemName_[128] = "NewCurve";
	char renameBuffer_[128] = "";
	char newTrackName_[128] = "Position";
	int newTrackDimension_ = 3;
	bool requestDeletePopup_ = false;

	// 自動保存
	float autoSaveTimer_ = 0.0f;
	std::string statusMessage_;
	float statusTimer_ = 0.0f;

	static constexpr size_t kMaxUndo = 100;
	static constexpr float kAutoSaveInterval = 2.0f;
	static inline const std::string kCacheSignature = "CurveCache";
};
