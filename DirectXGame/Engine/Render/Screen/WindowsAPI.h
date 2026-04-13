#pragma once
#include <Windows.h>
#include <Core/DXDevice.h>
#include <Core/Command/CommandManager.h>

namespace SHEngine::Screen {

	/**
	 * @class WindowsAPI
	 * @brief Windows APIを管理するクラス
	 *
	 * ウィンドウの作成、メッセージループの管理、入力処理など、
	 * Windows APIに関連する機能を提供する。
	 */
	class WindowsAPI {
	public:

		~WindowsAPI();

		struct WindowDesc {
			int width;					///< ウィンドウの幅
			int height;					///< ウィンドウの高さ
			std::wstring windowName;	///< ウィンドウのタイトル
			std::function<LRESULT(HWND, UINT, WPARAM, LPARAM)> wndProc;			///< ウィンドウプロシージャの関数ポインタ
		};
		void Initialize(WindowDesc& desc, HINSTANCE hInstance);

		static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

		bool IsCloseButtonPushed() const { return isPushCloseButton_; }

		HWND GetHwnd() const { return hwnd_; }
		std::pair<int, int> GetWindowSize() const { return { width_, height_ }; }

	private:

		HWND hwnd_ = nullptr;	///< ウィンドウハンドル
		WNDCLASS wc_;

		WindowDesc desc_;		///< ウィンドウの説明
		Logger logger_;			///< ロガー

		int width_ = 0;			///< ウィンドウの幅
		int height_ = 0;		///< ウィンドウの高さ

		bool isPushCloseButton_ = false;	///< ウィンドウのクローズボタンが押されたかどうか
	};
}
