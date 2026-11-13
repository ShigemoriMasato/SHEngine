#pragma once
#include <Tool/Binary/BinaryManager.h>
#include <Utility/Vector.h>
#include <cstdint>
#include <stdexcept>
#include <string>

// ゲーム設定(Config)をBinaryManagerで保存・読み込みする共通処理。
// 各Configは次の3関数を持つ。
//   void Save(BinaryManager&) const : 先頭に kVersion(uint32_t)、続けて全フィールドを固定順で Register する
//   bool Load(BinaryManager&)       : Save と同じ順で Reverse する。バージョン不一致なら false
//   void Validate()                 : NaN/Inf は宣言時の既定値へ戻し、範囲外の値は許容範囲へ Clamp する
// 保存先は BinaryManager の基準パス(Assets/Binary/)直下。ファイル名は Config ごとに一意にする。
// 実行中状態(HP現在値・位置・速度・タイマー・コンテナ等)は保存しない。
namespace ConfigFile {

	/// @brief 設定を読み込む。ファイル欠損・バージョン違い・サイズ不一致の場合は config を変更せず false を返す(既定値を維持)
	template<typename T>
	bool Load(const std::string& fileName, T& config) {
		BinaryManager file;
		if (!file.Boot(fileName)) {
			return false;
		}

		// Config は可変長データを含まないため、既定値で書き出したサイズと一致しない場合は旧形式・破損とみなす。
		// (BinaryManager::Reverse は読み込み位置の末尾超過を検出しないため、読み込み前にサイズで弾く)
		BinaryManager probe;
		const T defaults{};
		defaults.Save(probe);
		if (file.GetInputRawData().size() != probe.GetOutputRawData().size()) {
			return false;
		}

		T loaded{};
		if (!loaded.Load(file)) {
			return false;
		}
		loaded.Validate();
		config = loaded;
		return true;
	}

	/// @brief 設定を保存する。書き込みに失敗した場合は false(例外は外へ出さない)
	template<typename T>
	bool Save(const std::string& fileName, const T& config) {
		BinaryManager file;
		config.Save(file);
		try {
			file.Write(fileName);
		} catch (const std::exception&) {
			return false;
		}
		return true;
	}

	/// @brief バージョン値を読み、一致するか判定する
	bool ReadVersion(BinaryManager& manager, uint32_t expected);

	/// @brief 非有限なら fallback、そうでなければ [minValue, maxValue] へ Clamp
	float Sanitize(float value, float minValue, float maxValue, float fallback);
	/// @brief 非有限成分を含むなら fallback
	Vector2 Sanitize(const Vector2& value, const Vector2& fallback);
	/// @brief 非有限なら fallback、そうでなければ各成分を [0, 1] へ Clamp
	Vector4 SanitizeColor(const Vector4& value, const Vector4& fallback);
	int32_t Sanitize(int32_t value, int32_t minValue, int32_t maxValue);
	uint32_t Sanitize(uint32_t value, uint32_t minValue, uint32_t maxValue);
}
