#include "AudioData.h"
#include <cassert>
#include <algorithm>
#include <cmath>

AudioData::AudioData(IXAudio2* xAudio) {
	xAudio_ = xAudio;
}

AudioData::~AudioData() {
	audioData_.clear();
	autoDataList_.clear();
	wfex_ = {};
}

void AudioData::Update() {
	for (size_t i = 0; i < autoDataList_.size(); i++) {
		if (autoDataList_[i]->IsPlaying() == false) {
			autoDataList_.erase(autoDataList_.begin() + i);
			i--;
		}
	}

	for (const auto& data : playDataList_) {
		data->SetVolume(-1.0f);
	}
}

void AudioData::StopAll() {
	autoDataList_.clear();
}

void AudioData::SetVolume(float volume) {
	volume_ = volume;
}

void AudioData::SetOverallVolume(float volume) {
	overallVolume_ = volume;
}

IXAudio2SourceVoice* AudioData::StartSourceVoice(int loopCount, float pitch) {
	IXAudio2SourceVoice* playSource;
	// MaxFrequencyRatioの既定値は2.0(1オクターブ上まで)なので、kMaxPitchまで上げられるよう指定する
	HRESULT hr = xAudio_->CreateSourceVoice(&playSource, &wfex_, 0, kMaxPitch);
	assert(SUCCEEDED(hr));

	XAUDIO2_BUFFER buffer = {};
	buffer.pAudioData = audioData_.data();
	buffer.Flags = XAUDIO2_END_OF_STREAM;
	buffer.AudioBytes = static_cast<UINT32>(audioData_.size());
	buffer.LoopCount = std::clamp(loopCount, 0, 255);

	hr = playSource->SubmitSourceBuffer(&buffer);
	assert(SUCCEEDED(hr));

	// 鳴り始めから狙った音程・音量になるよう、Startより前に設定する
	playSource->SetFrequencyRatio(std::clamp(pitch, kMinPitch, kMaxPitch));
	playSource->SetVolume(volume_ * overallVolume_);

	hr = playSource->Start();
	assert(SUCCEEDED(hr));

	return playSource;
}

void AudioData::Play(int loopCount, float pitch) {
	IXAudio2SourceVoice* playSource = StartSourceVoice(loopCount, pitch);

	autoDataList_.push_back(std::make_unique<PlayData>(playSource, this, false, pitch));
	playDataList_.push_back(autoDataList_.back().get());
}

std::unique_ptr<PlayData> AudioData::CustomPlay(int loopCount, float pitch) {
	IXAudio2SourceVoice* playSource = StartSourceVoice(loopCount, pitch);

	auto data = std::make_unique<PlayData>(playSource, this, true, pitch);
	playDataList_.push_back(data.get());

	return std::move(data);
}

float AudioData::SemitonesToPitch(float semitones) {
	return std::pow(2.0f, semitones / 12.0f);
}

void AudioData::ReleasePlayData(PlayData* data) {
	for (size_t i = 0; i < playDataList_.size(); i++) {
		if (playDataList_[i] == data) {
			playDataList_.erase(playDataList_.begin() + i);
			return;
		}
	}
	assert(false && "PlayData not found");
}
