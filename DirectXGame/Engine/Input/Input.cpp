#include "Input.h"
#include <cassert>
#include <algorithm>

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "xinput.lib")

using XC = XBoxController;

DIMOUSESTATE Input::mouseState = {};
DIMOUSESTATE Input::preMouseState = {};
BYTE Input::keyState[256] = {};
BYTE Input::preKeyState[256] = {};
bool Input::isInitialized_ = false;
bool Input::xBoxButtonFlug_[int(XBoxController::kButtomCount)];
Vector2 Input::xBoxStickState_[2];

void Input::Initialize(HINSTANCE hInstance) {
	if (isInitialized_) {
		return;
	}

	isInitialized_ = true;

	hInstance_ = hInstance;
	hwnd_ = nullptr;

	HRESULT hr;
	hr = DirectInput8Create(
		hInstance_, DIRECTINPUT_VERSION, IID_IDirectInput8,
		(void**)&directInput_, nullptr);
	assert(SUCCEEDED(hr));

	//Keyboard
	hr = directInput_->CreateDevice(GUID_SysKeyboard, &keyboard_, NULL);
	assert(SUCCEEDED(hr));

	hr = keyboard_->SetDataFormat(&c_dfDIKeyboard);
	assert(SUCCEEDED(hr));

	//Mouse
	hr = directInput_->CreateDevice(GUID_SysMouse, &mouse_, NULL);
	assert(SUCCEEDED(hr));

	hr = mouse_->SetDataFormat(&c_dfDIMouse);
	assert(SUCCEEDED(hr));
}

void Input::SetWindow(HWND hwnd) {
	if (hwnd_ == hwnd) {
		return;
	}

	hwnd_ = hwnd;

	if (hwnd_ == nullptr) {
		MouseKeyboardClear();
		return;
	}

	//HWNDが変わったら再設定
	HRESULT hr = keyboard_->Unacquire();
	assert(SUCCEEDED(hr));
	hr = keyboard_->SetCooperativeLevel(
		hwnd_, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert(SUCCEEDED(hr));

	hr = mouse_->Unacquire();
	assert(SUCCEEDED(hr));
	hr = mouse_->SetCooperativeLevel(
		hwnd_, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert(SUCCEEDED(hr));

}

void Input::Update() {
	DWORD dwResult = XInputGetState(0, &xBoxState_); // 0はコントローラー番号（最大4台）

	if (dwResult == ERROR_SUCCESS) {
		// コントローラーが接続されている
		SHORT lx = xBoxState_.Gamepad.sThumbLX; // 左スティックX軸
		SHORT ly = xBoxState_.Gamepad.sThumbLY; // 左スティックY軸
		xBoxButtonFlug_[int(XC::kLeftTrigger)] = xBoxState_.Gamepad.bLeftTrigger; // 左トリガー
		xBoxButtonFlug_[int(XC::kRightTrigger)] = xBoxState_.Gamepad.bRightTrigger; // 右トリガー

		for (int i = 0; i < 4; ++i) {
			//ABYXのボタン状態の取得
			xBoxButtonFlug_[i] = (xBoxState_.Gamepad.wButtons & (0x1000 << i)) != 0;

			//↑↓←→の方向キー状態の取得
			xBoxButtonFlug_[int(XC::kUp) + i] = (xBoxState_.Gamepad.wButtons & (0x0001 << i)) != 0;
		}

		xBoxButtonFlug_[int(XC::kStart)] = (xBoxState_.Gamepad.wButtons & XINPUT_GAMEPAD_START) != 0;
		xBoxButtonFlug_[int(XC::kSelect)] = (xBoxState_.Gamepad.wButtons & XINPUT_GAMEPAD_BACK) != 0;

		xBoxStickState_[0].x = float(xBoxState_.Gamepad.sThumbRX) / 32767.0f; // 右スティックX軸の正規化
		xBoxStickState_[0].y = float(xBoxState_.Gamepad.sThumbRY) / 32767.0f; // 右スティックX軸の正規化
		xBoxStickState_[1].x = float(xBoxState_.Gamepad.sThumbLX) / 32767.0f; // 左スティックX軸の正規化
		xBoxStickState_[1].y = float(xBoxState_.Gamepad.sThumbLY) / 32767.0f; // 左スティックX軸の正規化

	} else {
		ControllerClear();
	}

	//HWNDが必要な奴らなので、hwndがnullptrなら抜ける
	if (hwnd_ == nullptr) {
		MouseKeyboardClear();
		return;
	}

	HRESULT hr = keyboard_->Acquire();
	assert(SUCCEEDED(hr));
	std::memcpy(preKeyState, keyState, sizeof(keyState));
	hr = keyboard_->GetDeviceState(sizeof(keyState), keyState);
	assert(SUCCEEDED(hr));

	hr = mouse_->Acquire();
	assert(SUCCEEDED(hr));
	std::memcpy(&preMouseState, &mouseState, sizeof(DIMOUSESTATE));
	hr = mouse_->GetDeviceState(sizeof(DIMOUSESTATE), &mouseState);
	assert(SUCCEEDED(hr));
}

BYTE* Input::GetKeyState() {
	if (!isInitialized_) {
		return nullptr;
	}
	return keyState;
}

BYTE* Input::GetPreKeyState() {
	if(!isInitialized_) {
		return nullptr;
	}
	return preKeyState;
}

BYTE Input::GetKeyState(int keyCode) {
	if (!isInitialized_) {
		return {};
	}
	if (keyCode < 0 || keyCode >= 256) {
		return {}; // Invalid key code
	}
	return keyState[keyCode];
}

BYTE Input::GetPreKeyState(int keyCode) {
	if (!isInitialized_) {
		return {};
	}
	if (keyCode < 0 || keyCode >= 256) {
		return {}; // Invalid key code
	}
	return preKeyState[keyCode];
}

Vector2 Input::GetMouseMove() {
	if (!isInitialized_) {
		return {};
	}
	return Vector2(float(mouseState.lX), float(mouseState.lY));
}

float Input::GetMouseWheel() {
	if(!isInitialized_) {
		return 0.0f;
	}
	return float(mouseState.lZ);
}

BYTE* Input::GetMouseButtonState() {
	if (!isInitialized_) {
		return nullptr;
	}
	return mouseState.rgbButtons;
}

BYTE* Input::GetPreMouseButtonState() {
	if (!isInitialized_) {
		return nullptr;
	}
	return preMouseState.rgbButtons;
}

bool* Input::GetXBoxButtonState() {
	return xBoxButtonFlug_;
}

bool Input::GetXBoxButtonState(XBoxController button) {
	return xBoxButtonFlug_[int(button)];
}

Vector2 Input::GetXBoxStickState(int type) {
	return xBoxStickState_[type];
}

void Input::ControllerClear() {
	ZeroMemory(&xBoxState_, sizeof(xBoxState_));
	std::fill(std::begin(xBoxButtonFlug_), std::end(xBoxButtonFlug_), false);
	xBoxStickState_[0] = {};
	xBoxStickState_[1] = {};
}

void Input::MouseKeyboardClear() {
	std::fill(std::begin(keyState), std::end(keyState), 0);
	std::fill(std::begin(preKeyState), std::end(preKeyState), 0);
	ZeroMemory(&mouseState, sizeof(mouseState));
	ZeroMemory(&preMouseState, sizeof(preMouseState));
}
