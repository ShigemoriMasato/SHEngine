#include "CurveBinder.h"
#include "../CurveEditorLog.h"
#include <Camera/Camera.h>
#include <algorithm>

//==============================================================
// ポインタ版
//==============================================================

void CurveBinder::BindFloat(const std::string& name, float* target) {
	Bind(name, { 1,
		[target](float* out) { out[0] = *target; },
		[target](const float* in) { *target = in[0]; } });
}

void CurveBinder::BindVector2(const std::string& name, Vector2* target) {
	Bind(name, { 2,
		[target](float* out) { out[0] = target->x; out[1] = target->y; },
		[target](const float* in) { target->x = in[0]; target->y = in[1]; } });
}

void CurveBinder::BindVector3(const std::string& name, Vector3* target) {
	Bind(name, { 3,
		[target](float* out) { out[0] = target->x; out[1] = target->y; out[2] = target->z; },
		[target](const float* in) { target->x = in[0]; target->y = in[1]; target->z = in[2]; } });
}

void CurveBinder::BindVector4(const std::string& name, Vector4* target) {
	Bind(name, { 4,
		[target](float* out) { out[0] = target->x; out[1] = target->y; out[2] = target->z; out[3] = target->w; },
		[target](const float* in) { target->x = in[0]; target->y = in[1]; target->z = in[2]; target->w = in[3]; } });
}

//==============================================================
// getter / setter 版
//==============================================================

void CurveBinder::BindFloat(const std::string& name, std::function<float()> getter, std::function<void(float)> setter) {
	Bind(name, { 1,
		[getter](float* out) { out[0] = getter(); },
		[setter](const float* in) { setter(in[0]); } });
}

void CurveBinder::BindVector2(const std::string& name, std::function<Vector2()> getter, std::function<void(const Vector2&)> setter) {
	Bind(name, { 2,
		[getter](float* out) { const Vector2 v = getter(); out[0] = v.x; out[1] = v.y; },
		[setter](const float* in) { setter(Vector2(in[0], in[1])); } });
}

void CurveBinder::BindVector3(const std::string& name, std::function<Vector3()> getter, std::function<void(const Vector3&)> setter) {
	Bind(name, { 3,
		[getter](float* out) { const Vector3 v = getter(); out[0] = v.x; out[1] = v.y; out[2] = v.z; },
		[setter](const float* in) { setter(Vector3(in[0], in[1], in[2])); } });
}

void CurveBinder::BindVector4(const std::string& name, std::function<Vector4()> getter, std::function<void(const Vector4&)> setter) {
	Bind(name, { 4,
		[getter](float* out) { const Vector4 v = getter(); out[0] = v.x; out[1] = v.y; out[2] = v.z; out[3] = v.w; },
		[setter](const float* in) { setter(Vector4(in[0], in[1], in[2], in[3])); } });
}

//==============================================================

void CurveBinder::AddPostApply(std::function<void()> func) {
	if (func) {
		postApply_.push_back(std::move(func));
	}
}

void CurveBinder::Bind(const std::string& name, Binding binding) {
	if (bindings_.contains(name)) {
		CurveEditorLog::Get()->warn("CurveBinder: binding \"{}\" was overwritten", name);
	}
	bindings_[name] = std::move(binding);
}

void CurveBinder::Unbind(const std::string& name) {
	bindings_.erase(name);
	captured_.erase(name);
}

void CurveBinder::Clear() {
	bindings_.clear();
	postApply_.clear();
	captured_.clear();
}

void CurveBinder::Apply(const CurveAsset& asset, float time) const {
	bool applied = false;
	for (const CurveTrack& track : asset.GetTracks()) {
		auto it = bindings_.find(track.GetName());
		if (it == bindings_.end()) continue;

		const Binding& binding = it->second;
		// トラックに無い成分は現在値のまま残す
		Values values = {};
		binding.get(values.data());
		const int count = std::min(track.GetDimension(), binding.dimension);
		for (int i = 0; i < count; ++i) {
			values[i] = track.GetChannel(i).Evaluate(time);
		}
		binding.set(values.data());
		applied = true;
	}
	if (applied) {
		CallPostApply();
	}
}

int CurveBinder::GetDimension(const std::string& name) const {
	auto it = bindings_.find(name);
	return it == bindings_.end() ? 0 : it->second.dimension;
}

bool CurveBinder::GetCurrentValue(const std::string& name, Values& out) const {
	auto it = bindings_.find(name);
	if (it == bindings_.end()) {
		return false;
	}
	out = {};
	it->second.get(out.data());
	return true;
}

std::vector<std::string> CurveBinder::GetBindingNames() const {
	std::vector<std::string> names;
	names.reserve(bindings_.size());
	for (const auto& [name, binding] : bindings_) {
		names.push_back(name);
	}
	return names;
}

void CurveBinder::Capture() {
	captured_.clear();
	for (const auto& [name, binding] : bindings_) {
		Values values = {};
		binding.get(values.data());
		captured_[name] = values;
	}
}

void CurveBinder::Restore() {
	if (captured_.empty()) {
		return;
	}
	for (const auto& [name, values] : captured_) {
		auto it = bindings_.find(name);
		if (it != bindings_.end()) {
			it->second.set(values.data());
		}
	}
	captured_.clear();
	CallPostApply();
}

void CurveBinder::CallPostApply() const {
	for (const auto& func : postApply_) {
		func();
	}
}

//==============================================================
// CameraCurveBinder
//==============================================================

void CameraCurveBinder::Bind(CurveBinder& binder, Camera* camera,
	const std::string& positionTrack, const std::string& rotationTrack) {
	if (!camera) {
		CurveEditorLog::Get()->error("CameraCurveBinder: camera is null. \"{}\" / \"{}\" are not bound.", positionTrack, rotationTrack);
		return;
	}
	binder.BindVector3(positionTrack,
		[camera]() { return camera->GetPosition(); },
		[camera](const Vector3& v) { camera->SetPosition(v); });
	binder.BindVector3(rotationTrack,
		[camera]() { return camera->GetRotation(); },
		[camera](const Vector3& v) { camera->SetRotation(v); });
	binder.AddPostApply([camera]() { camera->MakeMatrix(); });
}
