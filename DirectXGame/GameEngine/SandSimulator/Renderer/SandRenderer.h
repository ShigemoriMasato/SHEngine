#pragma once
#include "../Data/SandData.h"
#include <Assets/Model/ModelData.h>
#include <Render/Renderer.h>

namespace Sand {

	//仮としてVertexShaderで作成する。あとでMeshShaderに切り替える
	class Renderer {
	public:

		Renderer(Sand::Particle* particleData, const ModelData* cubeModel);

		void SetInstance(uint32_t instanceNum) { renderer_->instanceNum_ = instanceNum; }

		void Draw(DCC* dcc);

		void SetCamera(const Camera* camera);

	private:

		void SetBufferToRenderer();

		std::unique_ptr<SHEngine::Renderer> renderer_ = nullptr;
		Sand::Particle* particleData_ = nullptr;

		const Camera* camera_ = nullptr;

		Logger logger_ = GetLogger("SandSimulator");
		bool isPutCameraErrorLog_ = false;
	};

}
