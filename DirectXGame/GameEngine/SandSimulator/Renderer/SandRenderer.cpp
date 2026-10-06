#include "SandRenderer.h"

using namespace Sand;

Renderer::Renderer(Sand::Particle* particleData, const ModelData* cubeModel) {
	particleData_ = particleData;

	renderer_ = std::make_unique<SHEngine::Renderer>(SHEngine::VertexType::Position, cubeModel->meshes.front());

	renderer_->SetVS("Engine/SandSimulator/Sand.VS.hlsl");
	renderer_->SetPS("Engine/SandSimulator/Sand.PS.hlsl");
}

void Renderer::Draw(DCC* dcc) {
	if (!camera_) {
		//初回だけログを出す
		if (!isPutCameraErrorLog_) {
			logger_->error("Renderer: Camera is not set. Please set the camera before drawing.");
			isPutCameraErrorLog_ = true;
		}
		return;
	}
	isPutCameraErrorLog_ = false;
	renderer_->Draw(dcc);
}

void Renderer::SetCamera(const Camera* camera) {
	if (camera == nullptr) {
		return;
	}

	camera_ = camera;
	SetBufferToRenderer();
	logger_->info("Renderer: Camera Set");
}

void Renderer::SetBufferToRenderer() {
	renderer_->ResetGPUBuffers();

	renderer_->SetGPUBuffers({ camera_->GetVPBuffer(), particleData_->scale }, ShaderType::VERTEX_SHADER, BufferType::CBV);
	renderer_->SetGPUBuffer(particleData_->position, ShaderType::VERTEX_SHADER, BufferType::SRV);

	renderer_->SetGPUBuffer(particleData_->color, ShaderType::PIXEL_SHADER, BufferType::CBV);
}
