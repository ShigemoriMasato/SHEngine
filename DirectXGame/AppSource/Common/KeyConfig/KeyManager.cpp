#include "KeyManager.h"
#include <algorithm>

void KeyManager::Initialize(SHEngine::Input* input, SHEngine::Screen::IDisplay* display) {
	input_ = input;
	display_ = display;
	keyHistory_.resize(kMaxHistory_);
	buttonHistory_.resize(kMaxHistory_);
	stickHistory_.resize(kMaxHistory_);
	mouseHistory_.resize(kMaxHistory_);

	for (int i = 0; i < int(Key::Count); ++i) {
		resultKeyFlags_[Key(i)] = false;
	}
}

void KeyManager::Update() {
	// ================- keyの初期化 -================
	for(auto& [action, flag] : resultKeyFlags_) {
		flag = false;
	}

	// ==========================- キーの状態の更新 -==========================	
	//一番古いものを削除して、新しいものを追加する
	keyHistory_.erase(keyHistory_.begin());
	keyHistory_.emplace_back();

	buttonHistory_.erase(buttonHistory_.begin());
	buttonHistory_.emplace_back();

	stickHistory_.erase(stickHistory_.begin());
	stickHistory_.emplace_back();

	mouseHistory_.erase(mouseHistory_.begin());
	mouseHistory_.emplace_back();

	//Keyの数だけループ
	for (const auto& [action, keys] : keyMap_) {
		//Keyに登録されているDIKの数だけループ
		for (const auto& [key, keyState] : keys) {
			//履歴を作成する
			keyHistory_.back()[key] = input_->GetKeyState(key);
		}
	}

	for (const auto& [action, buttons] : buttonMap_) {
		//Keyに登録されているDIKの数だけループ
		for (const auto& [button, buttonState] : buttons) {
			//履歴を作成する
			buttonHistory_.back()[button] = input_->GetXBoxButtonState(button);
		}
	}

	//履歴を作成する
	stickHistory_.back().first = input_->GetXBoxStickState(1);
	stickHistory_.back().second = input_->GetXBoxStickState(0);

	for (const auto& [action, buttons] : mouseMap_) {
		//Keyに登録されているDIKの数だけループ
		for (const auto& [button, buttonState] : buttons) {
			//履歴を作成する
			mouseHistory_.back()[button] = input_->GetMouseButtonState()[button];
		}
	}

	// ================- keyの最終的な状態の更新 -================
	for (const auto& [action, key] : keyMap_) {

		//Keyに登録されているDIKの数だけループ
		for (const auto& [dik, targetState] : key) {

			//最終的な状態
			bool state = false;

			switch (targetState) {
			case KeyState::None:
				state = !keyHistory_.back()[dik];
				break;
			case KeyState::Trigger:
				state = keyHistory_.back()[dik] && !keyHistory_[keyHistory_.size() - 2][dik];
				break;
			case KeyState::Hold:
				state = keyHistory_.back()[dik];
				break;
			case KeyState::Release:
				state = !keyHistory_.back()[dik] && keyHistory_[keyHistory_.size() - 2][dik];
				break;
			}

			//trueになったら登録してループを抜ける
			if (state) {
				resultKeyFlags_[action] = state;
				break;
			}

		}
	}

	for (const auto& [action, buttons] : buttonMap_) {

		//すでにtrueになっていたらスキップ
		if (resultKeyFlags_[action]) {
			continue;
		}

		//Keyに登録されているDIKの数だけループ
		for (const auto& [button, targetState] : buttons) {
			//最終的な状態
			bool state = false;
			switch (targetState) {
			case KeyState::None:
				state = !buttonHistory_.back()[button];
				break;
			case KeyState::Trigger:
				state = buttonHistory_.back()[button] && !buttonHistory_[buttonHistory_.size() - 2][button];
				break;
			case KeyState::Hold:
				state = buttonHistory_.back()[button];
				break;
			case KeyState::Release:
				state = !buttonHistory_.back()[button] && buttonHistory_[buttonHistory_.size() - 2][button];
				break;
			}
			//trueになったら登録してループを抜ける
			if (state) {
				resultKeyFlags_[action] = state;
				break;
			}
		}
	}

	for (const auto& [action, stick] : stickMap_) {

		//すでにtrueになっていたらスキップ
		if (resultKeyFlags_[action]) {
			continue;
		}

		for (const auto& info : stick) {

			//Stickの傾き具合	[Pre, Now]
			std::pair<float, float> stickState;

			Vector2 prevStick, nowStick;
			if(info.direction == Direction::Left) {
				prevStick = stickHistory_[stickHistory_.size() - 2].first;
				nowStick = stickHistory_.back().first;
			} else {
				prevStick = stickHistory_[stickHistory_.size() - 2].second;
				nowStick = stickHistory_.back().second;
			}

			switch (info.stickDirection) {
			case StickDirection::Right:
				stickState.first = prevStick.x;
				stickState.second = nowStick.x;
				break;
			case StickDirection::Left:
				stickState.first = -prevStick.x;
				stickState.second = -nowStick.x;
				break;
			case StickDirection::Up:
				stickState.first = prevStick.y;
				stickState.second = nowStick.y;
				break;
			case StickDirection::Down:
				stickState.first = -prevStick.y;
				stickState.second = -nowStick.y;
				break;
			}

			switch (info.state) {
			case KeyState::None:
				resultKeyFlags_[action] = stickState.second < info.toggleValue;
				break;
			case KeyState::Trigger:
				resultKeyFlags_[action] = stickState.second >= info.toggleValue && stickState.first < info.toggleValue;
				break;
			case KeyState::Hold:
				resultKeyFlags_[action] = stickState.second >= info.toggleValue;
				break;
			case KeyState::Release:
				resultKeyFlags_[action] = stickState.second < info.toggleValue && stickState.first >= info.toggleValue;
				break;
			}

			//trueになったら登録してループを抜ける
			if (resultKeyFlags_[action]) {
				break;
			}
		}

	}

	for (const auto& [action, buttons] : mouseMap_) {

		//すでにtrueになっていたらスキップ
		if (resultKeyFlags_[action]) {
			continue;
		}

		//Keyに登録されているDIKの数だけループ
		for (const auto& [button, targetState] : buttons) {
			//最終的な状態
			bool state = false;
			switch (targetState) {
			case KeyState::None:
				state = !mouseHistory_.back()[button];
				break;
			case KeyState::Trigger:
				state = mouseHistory_.back()[button] && !mouseHistory_[mouseHistory_.size() - 2][button];
				break;
			case KeyState::Hold:
				state = mouseHistory_.back()[button];
				break;
			case KeyState::Release:
				state = !mouseHistory_.back()[button] && mouseHistory_[mouseHistory_.size() - 2][button];
				break;
			}
			//trueになったら登録してループを抜ける
			if (state) {
				resultKeyFlags_[action] = state;
				break;
			}
		}
	}

	UpdateActiveDevice();
}

Vector2 KeyManager::GetCursorPos() const {
#ifdef USE_IMGUI
	return display_->GetCursorPos(input_->GetCursorPos());
#endif
	return input_->GetCursorPos();
}

void KeyManager::SetKey(Key action, uint8_t DIK, KeyState state) {
	//すでに登録されていたら何もしない
	for (const auto& [key, keyState] : keyMap_[action]) {
		if (key == DIK && keyState == state) {
			return;
		}
	}

	//登録されていなかったら登録する
	keyMap_[action].emplace_back(DIK, state);
}

void KeyManager::SetButton(Key action, XBoxController button, KeyState state) {
	buttonMap_[action].emplace_back(button, state);
}

void KeyManager::SetStick(Key action, bool isLeftStick, StickDirection direction, float toggleValue, KeyState state) {
	Direction dir = isLeftStick ? Direction::Left : Direction::Right;
	StickInfo stickInfo = { dir, direction, toggleValue, state };
	stickMap_[action].emplace_back(stickInfo);
}

void KeyManager::SetMouse(Key action, int mouseButton, KeyState state) {
	mouseMap_[action].emplace_back(mouseButton, state);
}

Vector2 KeyManager::GetLeftStick() const {
	return ApplyDeadZone(stickHistory_.back().first, leftStickDeadZone_);
}

Vector2 KeyManager::GetRightStick() const {
	return ApplyDeadZone(stickHistory_.back().second, rightStickDeadZone_);
}

void KeyManager::SetStickDeadZone(float leftDeadZone, float rightDeadZone) {
	leftStickDeadZone_ = std::clamp(leftDeadZone, 0.0f, 0.95f);
	rightStickDeadZone_ = std::clamp(rightDeadZone, 0.0f, 0.95f);
}

Vector2 KeyManager::ApplyDeadZone(const Vector2& stick, float deadZone) {
	float length = stick.Length();
	if (!(length > deadZone)) {
		return { 0.0f, 0.0f };
	}

	//デッドゾーンの外側を0～1へ再スケールする(傾き量は保つ)
	float scaledLength = std::min((length - deadZone) / (1.0f - deadZone), 1.0f);
	return stick * (scaledLength / length);
}

void KeyManager::UpdateActiveDevice() {
	// ================- コントローラー操作の有無 -================
	bool controllerUsed = false;
	for (const auto& [button, pressed] : buttonHistory_.back()) {
		if (pressed) {
			controllerUsed = true;
			break;
		}
	}
	if (stickHistory_.back().first.Length() > leftStickDeadZone_ ||
		stickHistory_.back().second.Length() > rightStickDeadZone_) {
		controllerUsed = true;
	}

	// ================- キーボード／マウス操作の有無 -================
	bool keyboardMouseUsed = false;
	for (const auto& [dik, pressed] : keyHistory_.back()) {
		if (pressed) {
			keyboardMouseUsed = true;
			break;
		}
	}
	if (!keyboardMouseUsed) {
		for (const auto& [button, pressed] : mouseHistory_.back()) {
			if (pressed) {
				keyboardMouseUsed = true;
				break;
			}
		}
	}
	if (!keyboardMouseUsed) {
		Vector2 mouseMove = input_->GetMouseMove();
		keyboardMouseUsed = mouseMove.x != 0.0f || mouseMove.y != 0.0f;
	}

	//両方同時に操作された場合はコントローラーを優先する。どちらも操作が無ければ現状維持
	if (controllerUsed) {
		activeDevice_ = InputDevice::Controller;
	} else if (keyboardMouseUsed) {
		activeDevice_ = InputDevice::KeyboardMouse;
	}
}
