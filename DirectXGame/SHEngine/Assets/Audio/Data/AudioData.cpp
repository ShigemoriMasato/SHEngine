#include "AudioData.h"
#include <cassert>
#include <algorithm>

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

void AudioData::Play(int loopCount) {
	IXAudio2SourceVoice* playSource;
	xAudio_->CreateSourceVoice(&playSource, &wfex_);

	XAUDIO2_BUFFER buffer = {};
	buffer.pAudioData = audioData_.data();
	buffer.Flags = XAUDIO2_END_OF_STREAM;
	buffer.AudioBytes = static_cast<UINT32>(audioData_.size());
	buffer.LoopCount = std::clamp(loopCount, 0, 255);

	HRESULT hr = playSource->SubmitSourceBuffer(&buffer);
	assert(SUCCEEDED(hr));
	hr = playSource->Start();
	assert(SUCCEEDED(hr));

	playSource->SetVolume(volume_ * overallVolume_);

	autoDataList_.push_back(std::make_unique<PlayData>(playSource, this, false));
	playDataList_.push_back(autoDataList_.back().get());
}

std::unique_ptr<PlayData> AudioData::CustomPlay(int loopCount) {
	IXAudio2SourceVoice* playSource;
	xAudio_->CreateSourceVoice(&playSource, &wfex_);

	XAUDIO2_BUFFER buffer = {};
	buffer.pAudioData = audioData_.data();
	buffer.Flags = XAUDIO2_END_OF_STREAM;
	buffer.AudioBytes = static_cast<UINT32>(audioData_.size());
	buffer.LoopCount = std::clamp(loopCount, 0, 255);

	HRESULT hr = playSource->SubmitSourceBuffer(&buffer);
	assert(SUCCEEDED(hr));
	hr = playSource->Start();
	assert(SUCCEEDED(hr));

	playSource->SetVolume(volume_ * overallVolume_);

	auto data = std::make_unique<PlayData>(playSource, this, true);
	playDataList_.push_back(data.get());

	return std::move(data);
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
