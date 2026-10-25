#include "ParticleRenderer.h"

void Particle::Renderer::Initialize(Data* data, SHEngine::BufferContainer* container) {
	cameraBuffer_ = container->Create(BufferType::CBV, sizeof(CameraData));

	meshRenderer_ = std::make_unique<SHEngine::MeshRenderer>();

	meshRenderer_->SetAS("Mesh/Particle/Quad.AS.hlsl");
	meshRenderer_->SetMS("Mesh/Particle/Quad.MS.hlsl");
	meshRenderer_->SetPS("Game/GPUParticle.PS.hlsl");

	meshRenderer_->SetGPUBuffers({ data->count }, ShaderType::AMPLIFICATION_SHADER, BufferType::CBV);

	meshRenderer_->SetGPUBuffers({ data->positions, data->scales, data->parentIndices, data->parents }, ShaderType::MESH_SHADER, BufferType::SRV);
	meshRenderer_->SetGPUBuffers({ data->count, cameraBuffer_ }, ShaderType::MESH_SHADER, BufferType::CBV);

	meshRenderer_->SetGPUBuffers({ data->colors }, ShaderType::PIXEL_SHADER, BufferType::SRV);
}

void Particle::Renderer::Update() {
	if (!camera_) {
		return;
	}

	cameraData_.vpMatrix = camera_->GetVPMatrix();
	cameraData_.billboardMatrix = camera_->GetBillboardMatrix();
	cameraBuffer_->CopyBuffer(&cameraData_, sizeof(CameraData));
}

void Particle::Renderer::Draw(DCC* dcc) {
	if (!camera_) {
		return;
	}
	meshRenderer_->Draw(dcc);
}

void Particle::Renderer::SetInstance(uint32_t instanceNum) {
	meshRenderer_->SetDispatchGroup(instanceNum / 1024 + 1, 1, 1);
}

void  Particle::Renderer::SetCamera(const Camera* camera) {
	camera_ = camera;
	Update();
}
