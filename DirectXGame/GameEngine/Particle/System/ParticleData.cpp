#include "ParticleData.h"

void Particle::Data::Initialize(SHEngine::BufferContainer* container, uint32_t particleCount) {
	positions = container->Create(BufferType::SRV_UAV, sizeof(Vector3), particleCount, BufferNum::Single);
	scales = container->Create(BufferType::SRV_UAV, sizeof(float), particleCount, BufferNum::Single);
	colors = container->Create(BufferType::SRV_UAV, sizeof(Vector4) / 2, particleCount, BufferNum::Single);
	parentIndices = container->Create(BufferType::SRV_UAV, sizeof(uint32_t), particleCount, BufferNum::Single);
	parents = container->Create(BufferType::SRV, sizeof(Matrix4x4), parentMatricesMaxNum);
	count = container->Create(BufferType::CBV, sizeof(uint32_t), 1);

	countNum = particleCount;
	count->CopyBuffer(&countNum, sizeof(uint32_t));
}

void Particle::FreeList::Initialize(SHEngine::BufferContainer* container, uint32_t capacityCount) {
	freeList = container->Create(BufferType::SRV_UAV, sizeof(uint32_t), capacityCount, BufferNum::Single);
	freeListCount = container->Create(BufferType::UAV, sizeof(uint32_t), 1, BufferNum::Single);
	capacity = container->Create(BufferType::CBV, sizeof(uint32_t), 1);

	this->capacityNum = capacityCount;
	capacity->CopyBuffer(&capacityNum, sizeof(uint32_t));
}
