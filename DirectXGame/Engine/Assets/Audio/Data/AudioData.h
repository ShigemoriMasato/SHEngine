#pragma once
#include <string>
#include <memory>
#include <vector>

#include "PlayData.h"

/**
 * @class AudioData
 * @brief 音声データの基底クラス
 * 
 * 音声ファイルの読み込み、再生、停止などの基本機能を提供する抽象クラス。
 * 派生クラスで各フォーマット固有の読み込みと再生処理を実装する。
 */
class AudioData {
public:

	AudioData(IXAudio2* xAudio);
	~AudioData();

	void Update();

	// @brief 鳴っている音をすべて止める。CustomPlayは止まらない。
	void StopAll();

	// @brief 音量を設定する。0.0f ~ 1.0fの範囲で指定する
	void SetVolume(float volume);

	// @brief 全体の音量を設定する。0.0f ~ 1.0fの範囲で指定する。全てのAudioDataに影響する
	static void SetOverallVolume(float volume);

	// @brief 音が終わるまで再生する。ループ再生はしない。途中で音量を変えられない。
	void Play(int loopCount = 0);

	// @brief 再生データを取得できる再生関数。再生後の音に干渉したい時用
	// @return 再生データのポインタ
	std::unique_ptr<PlayData> CustomPlay(int loopCount);

	// @brief 再生データを破棄する。デストラクタで呼ばれる。
	void ReleasePlayData(PlayData* data);

	// @brief 音声データの名前を取得する
	std::string GetName() const { return name_; }

	// @brief 音量を取得する。全体の音量 x AudioDataの音量で計算される
	float GetVolume() const { return overallVolume_ * volume_; }

protected:

	WAVEFORMATEX wfex_;					///< フォーマット
	std::vector<BYTE> audioData_;		///< 音声データのバイト列
	std::string name_;					///< 音声データの名前

private:

	IXAudio2* xAudio_;					///< XAudio2インスタンスへのポインタ

	float volume_ = 1.0f;				///< 音量(0.0f ~ 1.0f)
	static inline float overallVolume_ = 1.0f; ///< 全体の音量(0.0f ~ 1.0f)

	std::vector<std::unique_ptr<PlayData>> autoDataList_; ///< カスタム再生じゃない方の再生データのリスト。寿命管理用
	std::vector<PlayData*> playDataList_; ///< 現在使用中の再生データ

};
