#pragma once
#include <Particle/System/ParticleData.h>
#include <Render/MeshRenderer.h>
#include <Camera/Camera.h>

namespace Particle {

	class Renderer {
	public:

		void Initialize(Data* data, SHEngine::BufferContainer* container);
		void Update();
		void Draw(DCC* dcc);

		void SetInstance(uint32_t instanceNum);
		void SetCamera(const Camera* camera);

	private:

		const Camera* camera_;

		struct CameraData {
			Matrix4x4 vpMatrix;
			Matrix4x4 billboardMatrix;
		} cameraData_;
		SHEngine::GPUBuffer* cameraBuffer_ = nullptr;

		std::unique_ptr<SHEngine::MeshRenderer> meshRenderer_;

	};

}
