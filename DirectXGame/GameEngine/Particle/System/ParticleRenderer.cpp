#include "ParticleRenderer.h"

void Particle::Renderer::Initialize(Data* data, SHEngine::BufferContainer* container, bool isCube) {
	cameraBuffer_ = container->Create(BufferType::CBV, sizeof(CameraData));

	meshRenderer_ = std::make_unique<SHEngine::MeshRenderer>();

	if (isCube) {
		meshRenderer_->SetAS("Mesh/Particle/Cube.AS.hlsl");
		meshRenderer_->SetMS("Mesh/Particle/Cube.MS.hlsl");
	} else {
		meshRenderer_->SetAS("Mesh/Particle/Quad.AS.hlsl");
		meshRenderer_->SetMS("Mesh/Particle/Quad.MS.hlsl");
	}
	meshRenderer_->SetPS("Game/GPUParticle.PS.hlsl");

	meshRenderer_->SetASBuffers(BufferType::CBV, 0, { data->count });

	meshRenderer_->SetMSBuffers(BufferType::SRV, 0, { data->positions, data->scales, data->parentIndices, data->parents });
	meshRenderer_->SetMSBuffers(BufferType::CBV, 0, { data->count, cameraBuffer_ });

	meshRenderer_->SetPSBuffers(BufferType::SRV, 0, { data->colors });

	SetBlendMode(SHEngine::PSO::BlendStateID::Add);
}

void Particle::Renderer::SetPixelShader(const std::string& ps) {
	meshRenderer_->SetPS(ps);
}

void Particle::Renderer::SetPSBuffer(BufferType bufferType, int registerNumber, SHEngine::GPUBuffer* buffer) {
	if (!buffer) {
		return;
	}
	meshRenderer_->SetPSBuffer(bufferType, registerNumber, buffer);
}

void Particle::Renderer::SetPSBuffers(BufferType bufferType, int startRegister, const std::vector<SHEngine::GPUBuffer*>& buffers) {
	for (int i = 0; i < int(buffers.size()); ++i) {
		SetPSBuffer(bufferType, startRegister + i, buffers[i]);
	}
}

void Particle::Renderer::SetBlendMode(SHEngine::PSO::BlendStateID blendMode, int index) {
	if (index < 0) {
		for (int i = 0; i < kRenderTargetCount; ++i) {
			meshRenderer_->SetBlendState(blendMode, i);
		}
		return;
	}
	if (index < kRenderTargetCount) {
		meshRenderer_->SetBlendState(blendMode, index);
	}
}

void Particle::Renderer::SetUseTexture(bool useTexture) {
	meshRenderer_->SetUseTexture(useTexture);
}

void Particle::Renderer::SetSampler(uint32_t samplerFlag) {
	meshRenderer_->SetSampler(samplerFlag);
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
