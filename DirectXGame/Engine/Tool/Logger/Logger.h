#pragma once
#include <spdlog/spdlog.h>
#include <string>
#include "CustomLogSetting.h"

/**
 * @typedef Logger
 * @brief spdlogのloggerエイリアス
 */
using Logger = std::shared_ptr<spdlog::logger>;

/**
 * @enum LoggerFlag
 * @brief ロガーの動作フラグ
 */
enum LoggerFlag : uint32_t {
	None = 0,                 ///< フラグなし
	MakeFile = 1 << 0,        ///< ログファイルを作成
	UseDebugString = 1 << 1,  ///< デバッグ文字列を使用
	UseConsole = 1 << 2,      ///< コンソール出力を使用
};

/**
 * @brief ロガーを取得
 * 
 * 指定された名前とフラグでロガーを作成または取得する。
 * 
 * @param name ロガー名
 * @param flag ロガーフラグ（デフォルト: MakeFile | UseDebugString）
 * @return Logger
 */
Logger GetLogger(const std::string& name, uint32_t flag = MakeFile | UseDebugString);

