#pragma once
#include "Data/CameraEditorData.h"
#include <Camera/Camera.h>
#include <Input/Input.h>

class CameraEditor {
public:

	void Initialize(Camera* camera);
	void Update(SHEngine::Input* input, Camera* editorCamera, float deltaTime);

	void SetData(const CameraCurveData& data);
	const CameraCurveData& GetData() const { return data_; }

private:

	void CurveDraw();
	void ApplyCameraAt(float time);
	void CameraGizmo(Camera* editorCamera);

	enum class EditorMode {
		kStop,
		kPlay,
	} mode_ = EditorMode::kStop;

	CameraCurveData data_;
	Camera* camera_ = nullptr;

	float timer_ = 0.0f;
	float viewTimeMin_ = 0.0f;
	float viewTimeMax_ = 5.0f;
	float viewValueMin_ = -5.0f;
	float viewValueMax_ = 5.0f;
	bool curveVisible_[6] = { true, true, true, true, true, true };
	bool fitViewRequested_ = true;
	bool loop_ = true;
	float valuePerGrid_[2] = { 1.0f, 1.0f };
	float valueViewCenter_[2] = { 0.0f, 0.0f };
	int curveGroup_ = 0;
	int selectedCurve_ = 0;
	uint32_t selectedKeyId_ = UINT32_MAX;

	bool isGizmoActive_ = false;

	enum class DragTarget {
		kNone,
		kKey,
		kLeftHandle,
		kRightHandle,
	} dragTarget_ = DragTarget::kNone;

};
