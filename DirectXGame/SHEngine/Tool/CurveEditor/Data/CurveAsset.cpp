#include "CurveAsset.h"
#include <algorithm>
#include <cmath>

namespace {
	Vector2 CubicBezier(const Vector2& p0, const Vector2& p1, const Vector2& p2, const Vector2& p3, float t) {
		const float u = 1.0f - t;
		return p0 * (u * u * u) + p1 * (3.0f * u * u * t) +
			p2 * (3.0f * u * t * t) + p3 * (t * t * t);
	}

	bool KeyEquals(const CurveKey& a, const CurveKey& b) {
		return a.time == b.time && a.value == b.value &&
			a.leftHandle.x == b.leftHandle.x && a.leftHandle.y == b.leftHandle.y &&
			a.rightHandle.x == b.rightHandle.x && a.rightHandle.y == b.rightHandle.y;
	}
}

//==============================================================
// CurveChannel
//==============================================================

// CameraAnimationCurve::Evaluate と同じアルゴリズム
float CurveChannel::Evaluate(float time) const {
	if (keys_.empty()) {
		return 0.0f;
	}
	if (time <= keys_.front().time) {
		return keys_.front().value;
	}
	if (time >= keys_.back().time) {
		return keys_.back().value;
	}

	auto next = std::upper_bound(keys_.begin(), keys_.end(), time,
		[](float value, const CurveKey& key) { return value < key.time; });
	const CurveKey& b = *next;
	const CurveKey& a = *(next - 1);
	if (b.time - a.time <= 0.00001f) {
		return b.value;
	}

	const Vector2 p0 = { a.time, a.value };
	Vector2 p1 = p0 + a.rightHandle;
	const Vector2 p3 = { b.time, b.value };
	Vector2 p2 = p3 + b.leftHandle;
	p1.x = std::clamp(p1.x, p0.x, p3.x);
	p2.x = std::clamp(p2.x, p0.x, p3.x);
	if (p1.x > p2.x) {
		const float middle = (p1.x + p2.x) * 0.5f;
		p1.x = middle;
		p2.x = middle;
	}

	// ハンドルが横に動くとベジェのパラメータ t は時間と一致しないため、
	// X を二分探索で逆算してから Y を評価する
	float low = 0.0f;
	float high = 1.0f;
	for (int i = 0; i < 18; ++i) {
		const float t = (low + high) * 0.5f;
		if (CubicBezier(p0, p1, p2, p3, t).x < time) {
			low = t;
		} else {
			high = t;
		}
	}
	return CubicBezier(p0, p1, p2, p3, (low + high) * 0.5f).y;
}

uint32_t CurveChannel::AddKey(const CurveKey& key) {
	CurveKey added = key;
	for (auto& current : keys_) {
		if (std::abs(current.time - added.time) < 0.0001f) {
			added.id = current.id;
			current = added;
			return added.id;
		}
	}
	added.id = nextKeyId_++;
	keys_.push_back(added);
	SortKeys();
	return added.id;
}

void CurveChannel::PushKeyRaw(const CurveKey& key) {
	CurveKey added = key;
	added.id = nextKeyId_++;
	keys_.push_back(added);
}

bool CurveChannel::RemoveKey(uint32_t id) {
	auto it = std::find_if(keys_.begin(), keys_.end(),
		[id](const CurveKey& key) { return key.id == id; });
	if (it == keys_.end()) {
		return false;
	}
	keys_.erase(it);
	return true;
}

void CurveChannel::SortKeys() {
	std::stable_sort(keys_.begin(), keys_.end(), [](const CurveKey& a, const CurveKey& b) {
		return a.time < b.time;
	});
}

void CurveChannel::Clear() {
	keys_.clear();
	nextKeyId_ = 0;
}

CurveKey* CurveChannel::FindKey(uint32_t id) {
	for (auto& key : keys_) {
		if (key.id == id) return &key;
	}
	return nullptr;
}

const CurveKey* CurveChannel::FindKey(uint32_t id) const {
	for (const auto& key : keys_) {
		if (key.id == id) return &key;
	}
	return nullptr;
}

float CurveChannel::GetEndTime() const {
	return keys_.empty() ? 0.0f : keys_.back().time;
}

bool CurveChannel::operator==(const CurveChannel& other) const {
	if (keys_.size() != other.keys_.size()) return false;
	for (size_t i = 0; i < keys_.size(); ++i) {
		if (!KeyEquals(keys_[i], other.keys_[i])) return false;
	}
	return true;
}

//==============================================================
// CurveTrack
//==============================================================

CurveTrack::CurveTrack(const std::string& name, int dimension) : name_(name) {
	channels_.resize(static_cast<size_t>(std::clamp(dimension, 1, kMaxDimension)));
}

void CurveTrack::Evaluate(float time, float* out) const {
	for (size_t i = 0; i < channels_.size(); ++i) {
		out[i] = channels_[i].Evaluate(time);
	}
}

float CurveTrack::EvaluateFloat(float time) const {
	return channels_.empty() ? 0.0f : channels_[0].Evaluate(time);
}

Vector2 CurveTrack::EvaluateVector2(float time) const {
	float v[kMaxDimension] = {};
	Evaluate(time, v);
	return { v[0], v[1] };
}

Vector3 CurveTrack::EvaluateVector3(float time) const {
	float v[kMaxDimension] = {};
	Evaluate(time, v);
	return { v[0], v[1], v[2] };
}

Vector4 CurveTrack::EvaluateVector4(float time) const {
	float v[kMaxDimension] = { 0.0f, 0.0f, 0.0f, 1.0f };
	Evaluate(time, v);
	return { v[0], v[1], v[2], v[3] };
}

void CurveTrack::AddKeyAll(float time, const float* values, float handleLength) {
	CurveKey key;
	key.time = time;
	key.leftHandle = { -handleLength, 0.0f };
	key.rightHandle = { handleLength, 0.0f };
	for (size_t i = 0; i < channels_.size(); ++i) {
		key.value = values[i];
		channels_[i].AddKey(key);
	}
}

void CurveTrack::SortKeys() {
	for (auto& channel : channels_) {
		channel.SortKeys();
	}
}

float CurveTrack::GetEndTime() const {
	float end = 0.0f;
	for (const auto& channel : channels_) {
		end = std::max(end, channel.GetEndTime());
	}
	return end;
}

bool CurveTrack::operator==(const CurveTrack& other) const {
	return name_ == other.name_ && channels_ == other.channels_;
}

//==============================================================
// CurveAsset
//==============================================================

CurveTrack* CurveAsset::FindTrack(const std::string& name) {
	for (auto& track : tracks_) {
		if (track.GetName() == name) return &track;
	}
	return nullptr;
}

const CurveTrack* CurveAsset::FindTrack(const std::string& name) const {
	for (const auto& track : tracks_) {
		if (track.GetName() == name) return &track;
	}
	return nullptr;
}

CurveTrack* CurveAsset::AddTrack(const std::string& name, int dimension) {
	if (name.empty() || FindTrack(name)) {
		return nullptr;
	}
	tracks_.emplace_back(name, dimension);
	return &tracks_.back();
}

bool CurveAsset::RemoveTrack(const std::string& name) {
	auto it = std::find_if(tracks_.begin(), tracks_.end(),
		[&](const CurveTrack& track) { return track.GetName() == name; });
	if (it == tracks_.end()) {
		return false;
	}
	tracks_.erase(it);
	return true;
}

void CurveAsset::Sort() {
	float lastKeyTime = 0.0f;
	for (auto& track : tracks_) {
		track.SortKeys();
		lastKeyTime = std::max(lastKeyTime, track.GetEndTime());
	}
	duration = std::max(duration, lastKeyTime);
}

bool CurveAsset::operator==(const CurveAsset& other) const {
	return duration == other.duration && loop == other.loop && tracks_ == other.tracks_;
}
