#pragma once
#include "../Data/SandData.h"
#include <Assets/Model/ModelData.h>
#include <Render/Renderer.h>

//仮としてVertexShaderで作成する。あとでMeshShaderに切り替える
class SandRenderer {
public:

	SandRenderer(Sand::Particle* particleData, ModelData* cubeModel);

	void Draw(DCC* dcc);

	void SetCamera(Camera* camera);

private:

	void SetBufferToRenderer();

	std::unique_ptr<SHEngine::Renderer> renderer_ = nullptr;
	Sand::Particle* particleData_ = nullptr;

	Camera* camera_ = nullptr;

	Logger logger_ = GetLogger("SandSimulator");
	bool isPutCameraErrorLog_ = false;
};
