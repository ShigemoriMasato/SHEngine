#include "CurvePlayer.h"
#include "CurveEditorLog.h"
#include "Data/CurveAssetIO.h"
#include <algorithm>
#include <cmath>

bool CurvePlayer::Load(const std::string& categoryName, const std::string& itemName) {
	CurveAsset asset;
	if (!CurveAssetIO::Load(asset, categoryName, itemName)) {
		CurveEditorLog::Get()->error("CurvePlayer: failed to load \"{}/{}\". The previous curve is kept.", categoryName, itemName);
		return false;
	}
	SetAsset(asset);
	return true;
}

void CurvePlayer::SetAsset(const CurveAsset& asset) {
	asset_ = asset;
	time_ = 0.0f;
	isFinished_ = false;
}

void CurvePlayer::Stop() {
	isPlaying_ = false;
	isFinished_ = false;
	time_ = 0.0f;
	Apply();
}

void CurvePlayer::Seek(float time) {
	time_ = std::clamp(time, 0.0f, std::max(asset_.duration, 0.0f));
	Apply();
}

void CurvePlayer::Update(float deltaTime) {
	if (!isPlaying_) {
		return;
	}
	if (Advance(time_, deltaTime * speed_, asset_.duration, IsLoop())) {
		isPlaying_ = false;
		isFinished_ = true;
	}
	Apply();
}

void CurvePlayer::Apply() const {
	if (binder_) {
		binder_->Apply(asset_, time_);
	}
}

bool CurvePlayer::Advance(float& time, float deltaTime, float duration, bool loop) {
	if (duration <= 0.0f) {
		time = 0.0f;
		return !loop;
	}
	time += deltaTime;
	if (loop) {
		time = std::fmod(time, duration);
		if (time < 0.0f) time += duration;
		return false;
	}
	if (time >= duration) {
		time = duration;
		return true;
	}
	if (time <= 0.0f) {
		time = 0.0f;
		return deltaTime < 0.0f;
	}
	return false;
}
