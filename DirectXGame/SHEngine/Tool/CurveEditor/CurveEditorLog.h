#pragma once
#include <Tool/Logger/Logger.h>

namespace CurveEditorLog {

	/// @brief CurveEditor 関連(エディタ・入出力・プレイヤー・バインダー)共通のロガー。
	/// ファイル(Logs/CurveEditor/)とデバッグ出力に出る
	inline const Logger& Get() {
		static const Logger logger = GetLogger("CurveEditor");
		return logger;
	}

}
