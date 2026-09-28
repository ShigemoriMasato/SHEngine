#include "PlayData.h"
#include "AudioData.h"
#include <cassert>

PlayData::PlayData(IXAudio2SourceVoice* playSource, AudioData* parentData, bool isCustom) {
	playSource_ = playSource;
	parentData_ = parentData;
	isCustom_ = isCustom;
}

PlayData::~PlayData() {
	parentData_->ReleasePlayData(this);
}

void PlayData::Play(bool isLoop) {
	if (IsPlaying()) {
		return;
	}

	HRESULT hr = playSource_->Start();
	assert(SUCCEEDED(hr));
}

void PlayData::Pause() {
	HRESULT hr = playSource_->Stop();
	assert(SUCCEEDED(hr));
}

void PlayData::Stop() {
	HRESULT hr = playSource_->Stop();
	assert(SUCCEEDED(hr));
	hr = playSource_->FlushSourceBuffers();
	assert(SUCCEEDED(hr));
}

bool PlayData::IsPlaying() const {
	XAUDIO2_VOICE_STATE state;
	playSource_->GetState(&state);
	return state.BuffersQueued > 0;
}

void PlayData::SetVolume(float volume) {
	if (volume < 0.0f || volume > 1.0f) {

		if (isCustom_) {
			playSource_->SetVolume(volume * parentData_->GetVolume());
		} else {
			playSource_->SetVolume(parentData_->GetVolume());
		}

	} else {

		playSource_->SetVolume(volume * parentData_->GetVolume());

	}
}
