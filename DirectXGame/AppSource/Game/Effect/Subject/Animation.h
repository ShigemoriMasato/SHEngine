#pragma once
#include <SHEngine.h>
#include <Render/Renderer.h>

class Animation_Sub {
public:

	void Initialize(SHEngine::Engine* engine);
	void Update(float deltaTime, const Matrix4x4& vpMatrix);
	void Draw(CmdObj* cmdObj);

private:

	SHEngine::Engine* engine_ = nullptr;

	std::unique_ptr<SHEngine::BufferContainer> container_;
	std::unique_ptr<SHEngine::Renderer> renderer_;

	SHEngine::GPUBuffer* wvpBuffer_ = nullptr;
	SHEngine::GPUBuffer* boneBuffer_ = nullptr;

	struct VSData {
		Matrix4x4 world;
		Matrix4x4 vp;
	}vsData;
	std::vector<WellForGPU> wellForGPU_;

	SkinningModelData modelData_;
	Animation animation_;
	float timer_ = 0.0f;
};
