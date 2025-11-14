#pragma once
#include <Windows.h>
#include <functional>
#include <string>

static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

class WindowsApp {
public:

	enum ShowType : uint32_t {
		kNormal = SW_SHOWNORMAL,
		kShow = SW_SHOW,
		kShowMax = SW_SHOWMAXIMIZED,
		kShowMin = SW_SHOWMINIMIZED,
		kRestore = SW_RESTORE,
		kHide = SW_HIDE,

		kCount
	};

public:

	WindowsApp() = default;
	~WindowsApp();

	void SetWindowProc(std::function<LRESULT(HWND, UINT, WPARAM, LPARAM)> windowProc);
	void SetSize(int32_t width, int32_t height);
	void SetWindowName(std::wstring name);
	void SetWindowClassName(std::wstring name);

	void Create();

	void Show(ShowType cmd = kShow);
	void Close();
	void Destroy();

	HWND GetHwnd() const { return hwnd_; }
	std::pair<int32_t, int32_t> GetWindowSize() const { return { clientWidth_, clientHeight_ }; }

private:

	//hwndを統括するためにstaticを使用
	HWND hwnd_;
	WNDCLASS wc_;

	std::wstring windowName_ = L"DirectXGame";
	std::wstring windowClassName_ = L"DirectXGameClass";

	std::function<LRESULT(HWND, UINT, WPARAM, LPARAM)> windowProc_;

	int32_t clientWidth_ = 1280;
	int32_t clientHeight_ = 720;

};
