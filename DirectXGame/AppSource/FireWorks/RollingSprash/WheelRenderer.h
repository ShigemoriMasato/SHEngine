#pragma once
#include <Render/Renderer.h>

class WheelRenderer {
public:

	WheelRenderer(ModelData* wheelModel);

	void Initialize();
	void Draw(DCC* dcc);

	void SetTransform(const std::vector<Matrix4x4>& transforms);
	void SetTransform(const Matrix4x4& transform, int index);

private:



};
