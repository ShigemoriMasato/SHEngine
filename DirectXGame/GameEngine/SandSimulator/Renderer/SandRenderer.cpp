#include "SandRenderer.h"

SandRenderer::SandRenderer(Sand::Particle* particleData, ModelData* cubeModel) {
	particleData_ = particleData;

	renderer_ = std::make_unique<SHEngine::Renderer>(SHEngine::VertexType::Position, cubeModel->meshes.front());

	renderer_->SetVS("Engine/SandSimulator/Sand.VS.hlsl");
	renderer_->SetPS("Engine/SandSimulator/Sand.PS.hlsl");

	SetBufferToRenderer();
}

void SandRenderer::Draw(DCC* dcc) {
	if (!camera_) {
		if (!isPutCameraErrorLog_) {
			logger_->error("Renderer: Camera is not set. Please set the camera before drawing.");
		}
		return;
	}
	isPutCameraErrorLog_ = true;
	renderer_->Draw(dcc);
}

void SandRenderer::SetCamera(Camera* camera) {
	camera_ = camera;
	SetBufferToRenderer();
	logger_->info("Renderer: Camera Set");
}

void SandRenderer::SetBufferToRenderer() {
	renderer_->ResetGPUBuffers();

	renderer_->SetGPUBuffers({ camera_->GetVPBuffer(), particleData_->scale }, ShaderType::VERTEX_SHADER, BufferType::CBV);
	renderer_->SetGPUBuffer(particleData_->position, ShaderType::PIXEL_SHADER, BufferType::SRV);

	renderer_->SetGPUBuffer(particleData_->color, ShaderType::PIXEL_SHADER, BufferType::CBV);
}
