#pragma once
#include "WindowsApp/WindowsApp.h"
#include "Display.h"
#include <Assets/Texture/TextureManager.h>

class SwapChain {
public:

	static void StaticInitialize(DXDevice* dxDevice, TextureManager* textureManager);

	SwapChain() = default;
	~SwapChain() = default;

	void CreateCommandObject();
	void Initialize(WindowsApp* createdApp, uint32_t color);

	void SwitchRenderTarget(bool isClear);
	void SwitchOffScreen(Display* display);

	void BeginCommand();
	ID3D12GraphicsCommandList* GetCommandList() const { return commandList_[index_].Get(); }
	void Present();

private:

	static DXDevice* dxDevice_;
	static TextureManager* textureManager_;

private:

	Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain_ = nullptr;

	std::array<Microsoft::WRL::ComPtr<ID3D12CommandAllocator>, 2> commandAllocator_;
	std::array<Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>, 2> commandList_;

	std::array<std::unique_ptr<Display>, 2> display_ = { nullptr, nullptr };
	bool isFirstDraw_[2] = { true, true };

	uint32_t index_ = 0;

private:

	std::shared_ptr<spdlog::logger> logger_ = nullptr;

};
