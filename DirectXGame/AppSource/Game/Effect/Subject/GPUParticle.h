#pragma once
#include <Compute/ComputeObject.h>
#include <Render/Renderer.h>

class GPUParticle {
public:

	void Initialize(CCC* ccc, const SHEngine::DrawData& drawData);
	void Update(CCC* ccc, const Matrix4x4& vpMatrix, const Matrix4x4& billboardMatrix, float deltaTime);
	void Draw(DCC* dcc);

private:

	struct Particle {
		Vector3 position;
		float lifeTime;

		Vector3 scale;
		float currentTime;

		Vector4 color;

		Vector3 velocity;
	};

	struct VSData {
		Matrix4x4 vpMatrix;
		Matrix4x4 billboardMatrix;
	};

	std::unique_ptr<SHEngine::BufferContainer> container_;

	std::unique_ptr<SHEngine::ComputeObject> initialize_;
	std::unique_ptr<SHEngine::ComputeObject> update_;
	std::unique_ptr<SHEngine::Renderer> renderer_;

	SHEngine::GPUBuffer* vsDataBuffer_ = nullptr;
};
