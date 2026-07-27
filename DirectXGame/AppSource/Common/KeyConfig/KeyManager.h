#pragma once
#include <Input/Input.h>
#include <unordered_map>
#include <Tool/Binary/BinaryManager.h>
#include <Common/MainDisplay.h>

enum class Key {
	// 移動系 -===========

	Right,
	Left,
	Up,
	Down,

	// 動作系 -===========

	HardDrop,
	Hold,

	// 回転系 -===========

	LRotate,
	RRotate,

	// UI系 -===========

	Correct,//確定キー
	Reverse,//取消キー
	Pause,//ポーズキー

	// Debug -===========

	Restart,//リスタートキー
	Debug1,
	Debug2,
	Debug3,

	// Editor用 -===========
	Z,
	Y,
	Delete,
	S,
	R,
	T,

	Count
};

enum class KeyState {
	None,
	Trigger,
	Hold,
	Release,

	Count
};

enum class StickDirection {
	Right,
	Left,
	Up,
	Down
};

//キーコンフィグの設定画面の作成は考慮してないです。よって削除はできません.
class KeyManager {
public:

	KeyManager() = default;
	~KeyManager() = default;

	void Initialize(SHEngine::Input* input, SHEngine::Screen::Display* display);
	void Update();

	std::unordered_map<Key, bool> GetKeyStates() const { return resultKeyFlags_; }
	Vector2 GetCursorPos() const;

	void SetKey(Key action, uint8_t DIK, KeyState state = KeyState::Release);
	void SetButton(Key action, XBoxController button, KeyState state);
	void SetStick(Key action, bool isLightStick, StickDirection direction, float toggleValue, KeyState state);
	//0.左クリック 1.右クリック 2.ホイールクリック
	void SetMouse(Key action, int mouseButton, KeyState state);

private:

	SHEngine::Screen::Display* display_ = nullptr;
	SHEngine::Input* input_ = nullptr;

	//スティックの種類
	enum Direction {
		Right,
		Left
	};

	struct StickInfo {
		Direction direction;
		StickDirection stickDirection;
		float toggleValue;
		KeyState state;
	};

	//Keyに割り当てられているDIKと状態
	std::unordered_map<Key, std::vector<std::pair<uint8_t, KeyState>>> keyMap_;
	std::unordered_map<Key, std::vector<std::pair<XBoxController, KeyState>>> buttonMap_;
	std::unordered_map<Key, std::vector<StickInfo>> stickMap_;
	std::unordered_map<Key, std::vector<std::pair<int, KeyState>>> mouseMap_;

	//キー入力の履歴を保存する。
	std::vector<std::unordered_map<uint8_t, bool>> keyHistory_;
	std::vector<std::unordered_map<XBoxController, bool>> buttonHistory_;
	std::vector<std::pair<Vector2, Vector2>> stickHistory_;		//Left, Right
	std::vector<std::unordered_map<int, bool>> mouseHistory_;
	//履歴の最大数
	const int kMaxHistory_ = 2;

	//最終的な状態を格納する
	std::unordered_map<Key, bool> resultKeyFlags_;
};
