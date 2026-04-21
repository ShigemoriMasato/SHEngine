#pragma once
#include <xaudio2.h>

class AudioData;

class PlayData {
public:

	PlayData(IXAudio2SourceVoice* playSource, AudioData* parentData, bool isCustom);
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

private:

	IXAudio2SourceVoice* playSource_;
	AudioData* parentData_; // このPlayDataを再生しているAudioDataへのポインタ。解放処理に使用

	bool isCustom_;

};
