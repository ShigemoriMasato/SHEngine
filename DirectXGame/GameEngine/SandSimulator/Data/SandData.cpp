#include "SandData.h"

void Sand::Particle::Initialize(SHEngine::BufferContainer* bufferContainer, uint32_t particleCount) {
	position = bufferContainer->Create(BufferType::SRV_UAV, sizeof(float) * 3, particleCount, BufferNum::Single);
	velocity = bufferContainer->Create(BufferType::SRV_UAV, sizeof(float) * 3, particleCount, BufferNum::Single);
	force = bufferContainer->Create(BufferType::SRV_UAV, sizeof(float) * 3, particleCount, BufferNum::Single);
	color = bufferContainer->Create(BufferType::CBV, sizeof(float) * 4, 1);
	scale = bufferContainer->Create(BufferType::CBV, sizeof(float), 1);
	count = bufferContainer->Create(BufferType::CBV, sizeof(int32_t), 1);

	count->CopyBuffer(&particleCount, sizeof(int32_t));
	countNum = particleCount;
}
