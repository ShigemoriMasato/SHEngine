#include "InitializeScene.h"
#include <imgui/imgui.h>

#ifdef USE_IMGUI
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif

void InitializeScene::Initialize() {
	auto& windowSet = commonData_->mainWindow;
	windowSet.first = std::make_unique<SHEngine::Screen::WindowsAPI>();
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

	windowSet.first->Initialize(desc, engine_->GetHInstance());
	windowSet.second = engine_->MakeWindow(windowSet.first.get(), 0x508888ff);

	commonData_->display = std::make_unique<SHEngine::Screen::MultiDisplay>();
	commonData_->display->Initialize(desc.width, desc.height, 0x050505FF, textureManager_);

	commonData_->cmdObject = engine_->CreateCommandObject(SHEngine::Command::Type::Direct);
	engine_->ImGuiActivate(windowSet.first.get(), commonData_->cmdObject.get());
}

std::unique_ptr<IScene> InitializeScene::Update() {
	//更新処理
	commonData_->cmdObject->ResetCommandList();

	return nullptr;
}

void InitializeScene::Draw() {
	auto swapChain = commonData_->mainWindow.second.get();
	auto display = commonData_->display.get();
	auto cmdObj = commonData_->cmdObject.get();

	display->ToPresent(cmdObj);
	display->ToPresent(cmdObj);

	swapChain->PreDraw(cmdObj);

	engine_->DrawImGui();
	swapChain->ToPresent(cmdObj);
}
