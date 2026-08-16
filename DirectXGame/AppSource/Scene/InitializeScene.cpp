#include "InitializeScene.h"
#include <imgui/imgui.h>
#include <Game/GameScene.h>
#include <Title/TitleScene.h>
#include <Test/TestScene.h>

#ifdef USE_IMGUI
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif

void InitializeScene::Initialize() {
	auto windowsAPI = std::make_unique<SHEngine::Screen::WindowsAPI>();
	SHEngine::Screen::WindowsAPI::WindowDesc desc;
	desc.width = 1280;
	desc.height = 720;
	desc.windowName = L"SHEngine";
	desc.wndProc = [&](HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) -> LRESULT {

#ifdef USE_IMGUI
		if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam)) {
			return true;
		}
#endif

		return DefWindowProc(hwnd, msg, wparam, lparam);
		};

	windowsAPI->Initialize(desc, engine_->GetHInstance());
	commonData_->window = std::make_unique<SHEngine::Screen::SwapChain>();
	commonData_->window->Initialize(textureManager_, directContext_, 0xffffffff, std::move(windowsAPI));

	input_->SetWindow(commonData_->window->GetWindowsAPI()->GetHwnd());

	commonData_->display = std::make_unique<SHEngine::Screen::Display>();
	commonData_->display->Initialize(1280, 720, "MainWindow");
	commonData_->display->CreateDepthTexture(textureManager_);
	commonData_->display->AddRenderTarget(textureManager_, 0x0000ff);
	commonData_->display->AddRenderTarget(textureManager_, 0xff);

	commonData_->subDisplay = std::make_unique<SHEngine::Screen::Display>();
	commonData_->subDisplay->Initialize(1280, 720, "SubWindow");
	commonData_->subDisplay->CreateDepthTexture(textureManager_);
	commonData_->subDisplay->AddRenderTarget(textureManager_, 0xff);

	engine_->ImGuiActivate(commonData_->window->GetWindowsAPI());

	textureManager_->LoadAllTextures();

	fontLoader_->Load("851Gkktt_005.ttf", 128);
	fontLoader_->Load("ZenOldMincho-Medium.ttf");


	//KeyManager
#pragma region 長いので折りたたみ
	commonData_->keyManager = std::make_unique<KeyManager>();

	auto keyManager = commonData_->keyManager.get();
	keyManager->Initialize(input_, commonData_->display.get());

	//KeyConfigの設定
	keyManager->SetKey(Key::Right, DIK_D, KeyState::Hold);
	keyManager->SetKey(Key::Right, DIK_RIGHTARROW, KeyState::Hold);
	keyManager->SetButton(Key::Right, XBoxController::kRight, KeyState::Hold);
	keyManager->SetStick(Key::Right, true, StickDirection::Right, 0.5f, KeyState::Hold);

	keyManager->SetKey(Key::Left, DIK_A, KeyState::Hold);
	keyManager->SetKey(Key::Left, DIK_LEFTARROW, KeyState::Hold);
	keyManager->SetButton(Key::Left, XBoxController::kLeft, KeyState::Hold);
	keyManager->SetStick(Key::Left, true, StickDirection::Left, 0.5f, KeyState::Hold);

	keyManager->SetKey(Key::Up, DIK_W, KeyState::Hold);
	keyManager->SetKey(Key::Up, DIK_UPARROW, KeyState::Hold);
	keyManager->SetButton(Key::Up, XBoxController::kUp, KeyState::Hold);
	keyManager->SetStick(Key::Up, true, StickDirection::Up, 0.5f, KeyState::Hold);

	keyManager->SetKey(Key::Down, DIK_S, KeyState::Hold);
	keyManager->SetKey(Key::Down, DIK_DOWNARROW, KeyState::Hold);
	keyManager->SetButton(Key::Down, XBoxController::kDown, KeyState::Hold);
	keyManager->SetStick(Key::Down, true, StickDirection::Down, 0.5f, KeyState::Hold);

	//================================================================================

	keyManager->SetKey(Key::HardDrop, DIK_W, KeyState::Trigger);
	keyManager->SetKey(Key::HardDrop, DIK_SPACE, KeyState::Trigger);
	keyManager->SetKey(Key::HardDrop, DIK_UPARROW, KeyState::Trigger);
	keyManager->SetButton(Key::HardDrop, XBoxController::kUp, KeyState::Trigger);
	keyManager->SetStick(Key::HardDrop, false, StickDirection::Up, 0.5f, KeyState::Hold);

	keyManager->SetKey(Key::Hold, DIK_LSHIFT, KeyState::Trigger);
	keyManager->SetKey(Key::Hold, DIK_C, KeyState::Trigger);
	keyManager->SetKey(Key::Hold, DIK_H, KeyState::Trigger);
	keyManager->SetKey(Key::Hold, DIK_L, KeyState::Trigger);
	keyManager->SetKey(Key::Hold, DIK_RSHIFT, KeyState::Trigger);
	keyManager->SetButton(Key::Hold, XBoxController::kLeftShoulder, KeyState::Trigger);
	keyManager->SetButton(Key::Hold, XBoxController::kLeftTrigger, KeyState::Trigger);
	keyManager->SetButton(Key::Hold, XBoxController::kRightShoulder, KeyState::Trigger);
	keyManager->SetButton(Key::Hold, XBoxController::kLeftTrigger, KeyState::Trigger);

	//================================================================================

	keyManager->SetKey(Key::LRotate, DIK_Z, KeyState::Trigger);
	keyManager->SetKey(Key::LRotate, DIK_J, KeyState::Trigger);
	keyManager->SetButton(Key::LRotate, XBoxController::kX, KeyState::Trigger);

	keyManager->SetKey(Key::RRotate, DIK_X, KeyState::Trigger);
	keyManager->SetKey(Key::RRotate, DIK_K, KeyState::Trigger);
	keyManager->SetButton(Key::RRotate, XBoxController::kY, KeyState::Trigger);

	//================================================================================

	keyManager->SetKey(Key::Correct, DIK_RETURN, KeyState::Trigger);
	keyManager->SetKey(Key::Correct, DIK_SPACE, KeyState::Trigger);
	keyManager->SetKey(Key::Correct, DIK_Z, KeyState::Trigger);
	keyManager->SetButton(Key::Correct, XBoxController::kA, KeyState::Trigger);

	keyManager->SetKey(Key::Reverse, DIK_X, KeyState::Trigger);
	keyManager->SetButton(Key::Reverse, XBoxController::kB, KeyState::Trigger);

	keyManager->SetKey(Key::Pause, DIK_ESCAPE, KeyState::Trigger);
	keyManager->SetKey(Key::Pause, DIK_F1, KeyState::Trigger);
	keyManager->SetButton(Key::Pause, XBoxController::kStart, KeyState::Trigger);

	//================================================================================

	keyManager->SetKey(Key::Restart, DIK_R, KeyState::Trigger);
	keyManager->SetKey(Key::Restart, DIK_ESCAPE, KeyState::Trigger);
	keyManager->SetButton(Key::Restart, XBoxController::kSelect, KeyState::Trigger);

	keyManager->SetKey(Key::Z, DIK_Z, KeyState::Trigger);
	keyManager->SetKey(Key::Y, DIK_Y, KeyState::Trigger);
	keyManager->SetKey(Key::Delete, DIK_DELETE, KeyState::Trigger);
	keyManager->SetKey(Key::S, DIK_S, KeyState::Trigger);
	keyManager->SetKey(Key::R, DIK_R, KeyState::Trigger);
	keyManager->SetKey(Key::T, DIK_T, KeyState::Trigger);

	keyManager->SetKey(Key::One, DIK_1, KeyState::Trigger);
	keyManager->SetKey(Key::Two, DIK_2, KeyState::Trigger);
	keyManager->SetKey(Key::Three, DIK_3, KeyState::Trigger);
	keyManager->SetKey(Key::Four, DIK_4, KeyState::Trigger);
	keyManager->SetKey(Key::Five, DIK_5, KeyState::Trigger);
	keyManager->SetKey(Key::Six, DIK_6, KeyState::Trigger);
	keyManager->SetKey(Key::Seven, DIK_7, KeyState::Trigger);
	keyManager->SetKey(Key::Eight, DIK_8, KeyState::Trigger);
	keyManager->SetKey(Key::Nine, DIK_9, KeyState::Trigger);
	keyManager->SetKey(Key::Zero, DIK_0, KeyState::Trigger);

#ifndef SH_RELEASE
	keyManager->SetKey(Key::Debug1, DIK_F1, KeyState::Trigger);
	keyManager->SetKey(Key::Debug2, DIK_F2, KeyState::Trigger);
	keyManager->SetKey(Key::Debug3, DIK_F3, KeyState::Trigger);
#else
	keyManager->SetMouse(Key::Correct, 0, KeyState::Trigger);
#endif
#pragma endregion
}

std::unique_ptr<IScene> InitializeScene::Update() {
	//更新処理
	return std::make_unique<TitleScene>();
	return std::make_unique<GameScene>();
	return std::make_unique<TestScene>();
	return nullptr;
}

void InitializeScene::Draw() {
	auto swapChain = commonData_->window.get();
	auto display = commonData_->display.get();

	directContext_->SetRenderTarget(swapChain, true);
	engine_->DrawImGui();
	swapChain->ToPresent(directContext_);
}
