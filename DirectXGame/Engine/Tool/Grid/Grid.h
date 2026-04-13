#pragma once
#include <Render/RenderObject.h>

class Grid {
public:

	void Initialize(SHEngine::DrawDataManager* drawDataManager);
	void Update(Vector3 middle, const Matrix4x4& vpMatrix);
	void Draw(CmdObj* cmdObj);

private:

	std::unique_ptr<SHEngine::RenderObject> render_;

	const float length_ = 200.0f;
	const float interval_ = 1.0f;
	const int thickInterval_ = 5;
	const int lineNum_ = static_cast<int>(length_ / interval_) + 1;


	struct LineConfig {
		Vector3 start;
		float padding;
		Vector3 end;
		uint32_t color;
	};
	std::vector<LineConfig> vertical_;
	std::vector<LineConfig> horizontal_;

	std::vector<LineConfig> gpuBuffer_;
};
