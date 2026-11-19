#pragma once
#include <Tool/CurveEditor/CurveEditor.h>
#include <Tool/CurveEditor/CurvePlayer.h>

class DeltaTimeManager {
public:

	DeltaTimeManager();

	void Update(float deltaTime);
	void DrawImGui();

	void Boot(std::string itemName);

	const float GetDeltaTime() const { return deltaTime_; }

private:

	static inline const std::string kCategory_ = "DeltaTime";

	float ratio_ = 1.0f;
	float deltaTime_ = 0.0f;

	CurveBinder binder_;

	CurvePlayer player_;
	CurveEditor editor_{ kCategory_ };

};
