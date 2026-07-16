#pragma once
#include <WTypesbase.h>
#include <DbgHelp.h>
#include <Windows.h>
#include <strsafe.h>

namespace SHEngine::Func {

	static void CreateDump(EXCEPTION_POINTERS* exception) {
		//自国の取得
		SYSTEMTIME time;
		GetLocalTime(&time);

		//ファイルパス作成
		wchar_t filepath[MAX_PATH] = {};
		CreateDirectory(L"./Dumps", nullptr);
		StringCchPrintfW(filepath, MAX_PATH, L"./Dumps/%04d-%02d%02d-%02d%02d.dmp", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute);
		HANDLE dumpFileHandle = CreateFile(filepath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0);

		//クラッシュした場所と種類を取得
		DWORD processId = GetCurrentProcessId();
		DWORD threadID = GetCurrentThreadId();

		//設定情報を入力
		MINIDUMP_EXCEPTION_INFORMATION minidumpInformation{ 0 };
		minidumpInformation.ThreadId = threadID;
		minidumpInformation.ExceptionPointers = exception;
		minidumpInformation.ClientPointers = FALSE;

		//ダンプファイルの作成
		MiniDumpWriteDump(GetCurrentProcess(), processId, dumpFileHandle, MiniDumpNormal, &minidumpInformation, nullptr, nullptr);

		CloseHandle(dumpFileHandle);
	}

}
