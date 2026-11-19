#pragma once
#include "Data/CurveAsset.h"
#include "Binding/CurveBinder.h"

/// @brief 再生用ランタイム。CurveAsset と Binder を持ち、Update(deltaTime) で時間を進めて Apply する。
/// エディタ・ImGui が無くても動く。
class CurvePlayer {
public:

	/// @brief Assets/Binary/CurveEditor/<category>/<item>.bin を読み込む
	bool Load(const std::string& categoryName, const std::string& itemName);
	void SetAsset(const CurveAsset& asset);
	const CurveAsset& GetAsset() const { return asset_; }

	/// @brief 書き込み先。nullptr の場合は時間だけ進む
	void SetBinder(CurveBinder* binder) { binder_ = binder; }

	void Play() { isPlaying_ = true; isFinished_ = false; }
	void Pause() { isPlaying_ = false; }
	/// @brief 停止して 0 秒に戻す(値も 0 秒の値を書き込む)
	void Stop();
	void Seek(float time);

	/// @brief 再生中なら時間を進め、Binder に値を書き込む
	void Update(float deltaTime);
	/// @brief 現在時間の値を Binder に書き込む
	void Apply() const;

	void SetSpeed(float speed) { speed_ = speed; }
	float GetSpeed() const { return speed_; }
	float GetTime() const { return time_; }
	float GetDuration() const { return asset_.duration; }
	bool IsPlaying() const { return isPlaying_; }
	/// @brief ループしないアセットが最後まで再生されたか
	bool IsFinished() const { return isFinished_; }

	/// @brief ループ設定を上書きする(-1: アセットの設定に従う, 0: しない, 1: する)
	void SetLoopOverride(int loop) { loopOverride_ = loop; }

	/// @brief 時間を進める(エディタのプレビューと共通の処理)。終端に達して止まった場合 true
	static bool Advance(float& time, float deltaTime, float duration, bool loop);

private:

	bool IsLoop() const { return loopOverride_ < 0 ? asset_.loop : loopOverride_ != 0; }

	CurveAsset asset_;
	CurveBinder* binder_ = nullptr;

	float time_ = 0.0f;
	float speed_ = 1.0f;
	bool isPlaying_ = false;
	bool isFinished_ = false;
	int loopOverride_ = -1;
};
