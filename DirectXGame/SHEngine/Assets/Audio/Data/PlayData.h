#pragma once
#include <xaudio2.h>

class AudioData;

class PlayData {
public:

	PlayData(IXAudio2SourceVoice* playSource, AudioData* parentData, bool isCustom, float pitch = 1.0f);
	~PlayData();

	// 音声の再生を開始する。Pauseした場合は、その地点から再開する
	void Play(bool isLoop = false);
	
	// 中断する。次Playしたとき、中断した地点から再開する
	void Pause();

	// 停止。次プレイするとき、最初から再生される
	void Stop();

	// 再生されているかどうか
	bool IsPlaying() const;

	// 音量調整。全体の音量 x AudioDataの音量 x 音声ごとの音量(本関数で設定)で最終的な音量が決まる
	void SetVolume(float volume);

	// ピッチ(再生速度の倍率)を変更する。1.0fで原音, 2.0fで1オクターブ上。AudioData::kMinPitch ~ kMaxPitchに丸められる
	// 再生速度が変わるため、音の長さも 1 / pitch 倍になる
	void SetPitch(float pitch);

	// 最後に設定したピッチ倍率(丸め後)。XAudio2への反映は次の音声処理時なので、設定直後でもこの値を返す
	float GetPitch() const { return pitch_; }

private:

	IXAudio2SourceVoice* playSource_;
	AudioData* parentData_; // このPlayDataを再生しているAudioDataへのポインタ。解放処理に使用

	bool isCustom_;
	float pitch_ = 1.0f;

};
