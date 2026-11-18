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
	// @param pitch 再生速度の倍率(1.0fで原音, 2.0fで1オクターブ上, 0.5fで1オクターブ下)。kMinPitch ~ kMaxPitchに丸められる
	void Play(int loopCount = 0, float pitch = 1.0f);

	// @brief 再生データを取得できる再生関数。再生後の音に干渉したい時用
	// @param pitch 再生速度の倍率。再生後もPlayData::SetPitchで変更できる
	// @return 再生データのポインタ
	std::unique_ptr<PlayData> CustomPlay(int loopCount, float pitch = 1.0f);

	// @brief 半音単位の値をピッチ倍率に変換する。+12で2.0f(1オクターブ上), -12で0.5f(1オクターブ下)
	static float SemitonesToPitch(float semitones);

	// @brief 設定できるピッチ倍率の範囲。上限はSourceVoice作成時に確保する値で、大きいほどメモリを使う
	static constexpr float kMinPitch = XAUDIO2_MIN_FREQ_RATIO;
	static constexpr float kMaxPitch = 4.0f;

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

	// @brief SourceVoiceを作成し、音声データを積んで再生を開始する
	IXAudio2SourceVoice* StartSourceVoice(int loopCount, float pitch);

	IXAudio2* xAudio_;					///< XAudio2インスタンスへのポインタ

	float volume_ = 1.0f;				///< 音量(0.0f ~ 1.0f)
	static inline float overallVolume_ = 1.0f; ///< 全体の音量(0.0f ~ 1.0f)

	std::vector<std::unique_ptr<PlayData>> autoDataList_; ///< カスタム再生じゃない方の再生データのリスト。寿命管理用
	std::vector<PlayData*> playDataList_; ///< 現在使用中の再生データ

};
