#include <SHEngine.h>
#include <Display/SwapChain.h>

int WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	auto engine = std::make_unique<SHEngine>();
	engine->Initialize();

	auto window = std::make_unique<WindowsApp>();
	window->SetWindowName(L"DirectXGame");
	window->Create();
	window->Show(WindowsApp::kShow);

	auto swapChain = std::make_unique<SwapChain>();
	swapChain->CreateCommandObject();
	swapChain->Initialize(window.get(), 0xffffff);
	
	while (engine->IsLoop()) {

		//更新===

		engine->Update();


		//描画===
		engine->PreDraw();

		swapChain->SwitchRenderTarget(true);

		auto commandList = swapChain->GetCommandList();
		commandList->Close();
		ID3D12CommandList* commandLists[] = { commandList };
		engine->EndFrame(commandLists);
		swapChain->Present();
	}

	return 0;
}