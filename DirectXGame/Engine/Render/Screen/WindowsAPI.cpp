#include "WindowsAPI.h"
#include <unordered_map>
#include <Utility/ConvertString.h>

using namespace SHEngine::Screen;

namespace {
	std::unordered_map<HWND, WindowsAPI*> windowMap;	///< ウィンドウハンドルとWindowsAPIインスタンスのマップ
}

SHEngine::Screen::WindowsAPI::~WindowsAPI() {
	// ウィンドウの破棄
	if (hwnd_) {
		windowMap.erase(hwnd_);
		DestroyWindow(hwnd_);
		hwnd_ = nullptr;
	}
}

void SHEngine::Screen::WindowsAPI::Initialize(WindowDesc& desc, HINSTANCE hInstance) {
	logger_ = GetLogger("Engine");

	std::string logMsg = "Initializing WindowsAPI with width: " + std::to_string(desc.width) +
		", height: " + std::to_string(desc.height) +
		", windowName: " + ConvertString(desc.windowName);

	logger_->info(logMsg);

	// ウィンドウクラスの登録
	wc_.lpfnWndProc = WindowProc;
	wc_.lpszClassName = desc.windowName.c_str();
	wc_.hInstance = hInstance;
	wc_.hCursor = LoadCursor(nullptr, IDC_ARROW);

	RegisterClass(&wc_);

	RECT wrc = { 0, 0, desc.width, desc.height };
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	// ウィンドウの作成
	hwnd_ = CreateWindow(
		wc_.lpszClassName,
		wc_.lpszClassName,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		wrc.right - wrc.left, wrc.bottom - wrc.top,
		nullptr, nullptr, wc_.hInstance, nullptr
	);

	if (!hwnd_) {
		logger_->error("Failed to create window.");
		return;
	}

	// ウィンドウハンドルとWindowsAPIインスタンスのマップに登録
	windowMap[hwnd_] = this;

	desc_ = desc;
	width_ = desc.width;
	height_ = desc.height;

	ShowWindow(hwnd_, SW_SHOW);
}

LRESULT CALLBACK SHEngine::Screen::WindowsAPI::WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {

	auto it = windowMap.find(hwnd);
	if (it != windowMap.end()) {
		
		RECT rc;
		GetClientRect(hwnd, &rc);

		it->second->width_ = rc.right - rc.left;
		it->second->height_ = rc.bottom - rc.top;

		if (msg == WM_DESTROY) {
			it->second->logger_->info("Window is pushed Close Button.");
			it->second->isPushCloseButton_ = true;
			PostQuitMessage(0);
		}

		//WndProcが登録されていない場合はDefWindowProcを呼び出す
		if (!it->second->desc_.wndProc) {
			return DefWindowProc(hwnd, msg, wparam, lparam);
		}

		return it->second->desc_.wndProc(hwnd, msg, wparam, lparam);
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}
