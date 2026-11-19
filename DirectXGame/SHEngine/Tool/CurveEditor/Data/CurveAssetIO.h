#pragma once
#include "CurveAsset.h"
#include <string>
#include <vector>

class BinaryManager;

/// @brief CurveAsset の保存と読み込み(BinaryManager を使用)。エディタに依存しないのでゲーム本体からも使える。
///
/// 保存先: Assets/Binary/CurveEditor/<カテゴリー名>/<アイテム名>.bin
///
/// フォーマット(BinaryManager の Register 順):
///   string  "CurveAsset"(識別子)
///   uint32  formatVersion
///   float   duration
///   bool    loop
///   uint32  trackCount
///   [ string trackName, uint32 dimension,
///     [ uint32 keyCount, [ float time, float value, Vector2 leftHandle, Vector2 rightHandle ] × keyCount ] × dimension
///   ] × trackCount
class CurveAssetIO {
public:

	/// @brief 現在のフォーマットバージョン。項目を追加したら上げ、Read で分岐する
	static constexpr uint32_t kFormatVersion = 1;

	static inline const std::string kBinaryRoot = "Assets/Binary/";
	static inline const std::string kCurveRoot = "CurveEditor/";
	static inline const std::string kExtension = ".bin";
	static inline const std::string kCacheExtension = ".cache";

	/// @brief ゲーム本体から呼ぶ読み込み関数
	static bool Load(CurveAsset& outAsset, const std::string& categoryName, const std::string& itemName);
	/// @brief 保存する(ディレクトリが無ければ作る)
	static bool Save(const CurveAsset& asset, const std::string& categoryName, const std::string& itemName);

	/// @brief BinaryManager へ書き込む / から読み込む(他のデータと同じファイルに入れたい場合用)
	static void Write(BinaryManager& bin, const CurveAsset& asset);
	static bool Read(BinaryManager& bin, CurveAsset& outAsset);

	/// @brief カテゴリー内のアイテム名一覧(拡張子 .bin のファイル名)
	static std::vector<std::string> ListItems(const std::string& categoryName);

	/// @brief ファイル名に使えない文字を '_' に置き換える。空なら "NewCurve"
	static std::string SanitizeName(const std::string& name);

	/// @brief BinaryManager に渡す相対パス(CurveEditor/<カテゴリー>/<アイテム><拡張子>)
	static std::string GetRelativePath(const std::string& categoryName, const std::string& itemName,
		const std::string& extension = kExtension);
	/// @brief 実パス(Assets/Binary/CurveEditor/<カテゴリー>/)
	static std::string GetDirectory(const std::string& categoryName);
	/// @brief ディレクトリを作る(BinaryManager は Assets/Binary/ 直下しか作らないため)
	static bool EnsureDirectory(const std::string& categoryName);

private:

	static inline const std::string kSignature = "CurveAsset";
};
