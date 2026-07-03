#pragma once
#include <Render/Renderer.h>

class Grid {
public:

	void Initialize(SHEngine::DrawDataManager* drawDataManager);
	void Update(Vector3 middle, const Matrix4x4& vpMatrix);
	void Draw(DCC* dcc);

private:

	std::unique_ptr<SHEngine::BufferContainer> container_;
	std::unique_ptr<SHEngine::Renderer> renderer_;

	SHEngine::GPUBuffer* configBuffer_ = nullptr;
	SHEngine::GPUBuffer* vpBuffer_ = nullptr;

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

	std::vector<LineConfig> configs_;
};
