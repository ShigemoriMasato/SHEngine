#pragma once
#include <Input/Input.h>
#include <unordered_map>
#include <Tool/Binary/BinaryManager.h>
#include <Render/Screen/IDisplay.h>
#include "InputDevice.h"

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

	// ゲーム操作系 -===========

	Fire,//射撃キー(Hold)。UI決定のCorrectとは独立させる

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

	One,
	Two,
	Three,
	Four,
	Five,
	Six,
	Seven,
	Eight,
	Nine,
	Zero,

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

	void Initialize(SHEngine::Input* input, SHEngine::Screen::IDisplay* display);
	void Update();

	std::unordered_map<Key, bool> GetKeyStates() const { return resultKeyFlags_; }
	Vector2 GetCursorPos() const;

	void SetKey(Key action, uint8_t DIK, KeyState state = KeyState::Release);
	void SetButton(Key action, XBoxController button, KeyState state);
	void SetStick(Key action, bool isLeftStick, StickDirection direction, float toggleValue, KeyState state);
	//0.左クリック 1.右クリック 2.ホイールクリック
	void SetMouse(Key action, int mouseButton, KeyState state);

	// ================- アナログ入力 -================
	// GetKeyStatesの方向boolはメニュー操作用にそのまま残し、ゲーム操作用のアナログ値はこちらで取得する。

	/// @brief デッドゾーン適用後の左スティック値。x:右が正 y:上が正、長さは0～1。
	/// デッドゾーン外側は (長さ - デッドゾーン) / (1 - デッドゾーン) に再スケールし、傾き量を保つ(一律に長さ1へ正規化しない)
	Vector2 GetLeftStick() const;
	/// @brief デッドゾーン適用後の右スティック値。仕様はGetLeftStickと同じ
	Vector2 GetRightStick() const;
	/// @brief スティックのデッドゾーン(0～0.95)。右スティックのデッドゾーンは操作デバイスの切り替え判定にも使う
	void SetStickDeadZone(float leftDeadZone, float rightDeadZone);

	/// @brief 最後に有効な操作を行ったデバイス。両方同時に操作された場合はコントローラーを優先する
	InputDevice GetActiveDevice() const { return activeDevice_; }

private:

	SHEngine::Screen::IDisplay* display_ = nullptr;
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

	//操作デバイスの判定
	void UpdateActiveDevice();
	static Vector2 ApplyDeadZone(const Vector2& stick, float deadZone);

	float leftStickDeadZone_ = 0.2f;
	float rightStickDeadZone_ = 0.25f;
	InputDevice activeDevice_ = InputDevice::Controller;
};
