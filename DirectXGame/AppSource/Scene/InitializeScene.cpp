#include "InitializeScene.h"
#include <imgui/imgui.h>
#include <Game/GameScene.h>
#include <Title/TitleScene.h>
#include <Test/TestScene.h>
#include <Editor/EditScene.h>
#include <EffectTest/EffectTestScene.h>

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
	commonData_->display->AddRenderTarget(textureManager_, 0xff);
	commonData_->display->AddRenderTarget(textureManager_, 0xff);

	std::vector<VertexData> vertices = {
		{{-1.0f, 1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}},
		{{3.0f, 1.0f, 0.0f, 1.0f}, {2.0f, 0.0f}, {0.0f, 0.0f, -1.0f}},
		{{-1.0f, -3.0f, 0.0f, 1.0f}, {0.0f, 2.0f}, {0.0f, 0.0f, -1.0f}}
	};
	drawDataManager_->AddVertexBuffer(vertices);
	commonData_->postEffectDrawDataIndex = drawDataManager_->CreateDrawData();

	engine_->ImGuiActivate(commonData_->window->GetWindowsAPI());

	textureManager_->LoadAllTextures();

	fontLoader_->Load("YDWbananaslipplus.otf", 64);

	//KeyManager
#pragma region 長いので折りたたみ
	commonData_->keyManager = std::make_unique<KeyManager>();

	auto keyManager = commonData_->keyManager.get();
	keyManager->Initialize(input_, commonData_->display.get());

	//KeyConfigの設定
	keyManager->SetKey(Key::Right, DIK_D, KeyState::Hold);
	keyManager->SetKey(Key::Right, DIK_RIGHTARROW, KeyState::Hold);
	keyManager->SetButton(Key::Right, XBoxController::kRight, KeyState::Hold);
	keyManager->SetStick(Key::Right, false, StickDirection::Right, 0.5f, KeyState::Hold);

	keyManager->SetKey(Key::Left, DIK_A, KeyState::Hold);
	keyManager->SetKey(Key::Left, DIK_LEFTARROW, KeyState::Hold);
	keyManager->SetButton(Key::Left, XBoxController::kLeft, KeyState::Hold);
	keyManager->SetStick(Key::Left, false, StickDirection::Left, 0.5f, KeyState::Hold);

	keyManager->SetKey(Key::Up, DIK_W, KeyState::Hold);
	keyManager->SetKey(Key::Up, DIK_UPARROW, KeyState::Hold);
	keyManager->SetButton(Key::Up, XBoxController::kUp, KeyState::Hold);
	keyManager->SetStick(Key::Up, false, StickDirection::Up, 0.5f, KeyState::Hold);

	keyManager->SetKey(Key::Down, DIK_S, KeyState::Hold);
	keyManager->SetKey(Key::Down, DIK_DOWNARROW, KeyState::Hold);
	keyManager->SetButton(Key::Down, XBoxController::kDown, KeyState::Hold);
	keyManager->SetStick(Key::Down, false, StickDirection::Down, 0.5f, KeyState::Hold);

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

	keyManager->SetKey(Key::Debug1, DIK_F1, KeyState::Trigger);
	keyManager->SetKey(Key::Debug2, DIK_F2, KeyState::Trigger);
	keyManager->SetKey(Key::Debug3, DIK_F3, KeyState::Trigger);
#pragma endregion
}

std::unique_ptr<IScene> InitializeScene::Update() {
	//更新処理
	return std::make_unique<EffectTestScene>();
	return std::make_unique<EditScene>();
	return std::make_unique<TestScene>();
	return std::make_unique<GameScene>();

	return nullptr;
}

void InitializeScene::Draw() {
	auto swapChain = commonData_->window.get();
	auto display = commonData_->display.get();

	directContext_->SetRenderTarget(swapChain, true);
	engine_->DrawImGui();
	swapChain->ToPresent(directContext_);
}
