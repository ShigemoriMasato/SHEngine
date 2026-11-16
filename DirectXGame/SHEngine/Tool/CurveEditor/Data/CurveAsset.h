#pragma once
#include <Utility/DataStructures.h>
#include <string>
#include <vector>

// ImGuiに依存しないカーブのデータ層。ゲーム本体(リリースビルド)からも使用する。
// キーは CameraEditor と同じ CurveKey(time, value, ベジェハンドル)を使用し、
// 評価も CameraAnimationCurve::Evaluate と同じアルゴリズムで行う。

struct CurveKey {
	float value = 0.0f;
	float time = 0.0f;

	Vector2 leftHandle = { -0.2f, 0.0f };
	Vector2 rightHandle = { 0.2f, 0.0f };

	uint32_t id = 0;
};

/// @brief float 1本分のキー列
class CurveChannel {
public:

	/// @brief 時間(秒)での値を評価する。キーが無い場合は 0
	float Evaluate(float time) const;

	/// @brief キーを追加する。同じ時間(誤差0.0001秒)にキーがある場合は上書きする
	/// @return 追加(上書き)されたキーのID
	uint32_t AddKey(const CurveKey& key);
	/// @brief 読み込み用。時間の重複チェックをせずに末尾へ追加する
	void PushKeyRaw(const CurveKey& key);
	bool RemoveKey(uint32_t id);
	void SortKeys();
	void Clear();

	CurveKey* FindKey(uint32_t id);
	const CurveKey* FindKey(uint32_t id) const;
	std::vector<CurveKey>& GetKeys() { return keys_; }
	const std::vector<CurveKey>& GetKeys() const { return keys_; }

	/// @brief 最後のキーの時間(キーが無い場合は 0)
	float GetEndTime() const;

	bool operator==(const CurveChannel& other) const;

private:

	std::vector<CurveKey> keys_;
	uint32_t nextKeyId_ = 0;
};

/// @brief 名前と次元数(1〜4)を持つ値1つ分のカーブ
class CurveTrack {
public:

	static constexpr int kMaxDimension = 4;

	CurveTrack() = default;
	CurveTrack(const std::string& name, int dimension);

	const std::string& GetName() const { return name_; }
	void SetName(const std::string& name) { name_ = name; }
	int GetDimension() const { return static_cast<int>(channels_.size()); }

	CurveChannel& GetChannel(int index) { return channels_[index]; }
	const CurveChannel& GetChannel(int index) const { return channels_[index]; }

	/// @brief 全チャンネルを評価して out[0..dimension) に書き込む
	void Evaluate(float time, float* out) const;
	float EvaluateFloat(float time) const;
	Vector2 EvaluateVector2(float time) const;
	Vector3 EvaluateVector3(float time) const;
	Vector4 EvaluateVector4(float time) const;

	/// @brief 全チャンネルの同じ時間にキーを打つ(values は dimension 個)
	void AddKeyAll(float time, const float* values, float handleLength = 0.2f);

	void SortKeys();
	float GetEndTime() const;

	bool operator==(const CurveTrack& other) const;

private:

	std::string name_;
	std::vector<CurveChannel> channels_;
};

/// @brief アイテム1つ分のデータ。複数の Track と全体の長さ、ループ設定を持つ
class CurveAsset {
public:

	/// @brief 全体の長さ(秒)
	float duration = 1.0f;
	bool loop = true;

	std::vector<CurveTrack>& GetTracks() { return tracks_; }
	const std::vector<CurveTrack>& GetTracks() const { return tracks_; }

	CurveTrack* FindTrack(const std::string& name);
	const CurveTrack* FindTrack(const std::string& name) const;

	/// @brief Track を追加する。同名が既にある場合は nullptr
	/// @note 追加すると以前に取得した CurveTrack のポインタは無効になる
	CurveTrack* AddTrack(const std::string& name, int dimension);
	bool RemoveTrack(const std::string& name);

	/// @brief 全キーをソートし、duration を最後のキー以上にする(CameraCurveData::Sort と同じ)
	void Sort();

	bool operator==(const CurveAsset& other) const;

private:

	std::vector<CurveTrack> tracks_;
};
