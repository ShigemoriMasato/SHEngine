#include "ParticleData.h"

void Particle::Data::Initialize(SHEngine::BufferContainer* container, uint32_t particleCount) {
	positions = container->Create(BufferType::SRV_UAV, sizeof(Vector3), particleCount, BufferNum::Single, "Positions");
	rotations = container->Create(BufferType::SRV_UAV, sizeof(Vector3), particleCount, BufferNum::Single, "Rotations");
	scales = container->Create(BufferType::SRV_UAV, sizeof(float), particleCount, BufferNum::Single, "Scales");
	colors = container->Create(BufferType::SRV_UAV, sizeof(Vector4) / 2, particleCount, BufferNum::Single, "Colors");
	parentIndices = container->Create(BufferType::SRV_UAV, sizeof(uint32_t), particleCount, BufferNum::Single, "ParentIndices");
	parents = container->Create(BufferType::SRV, sizeof(Matrix4x4), parentMatricesMaxNum, BufferNum::MatchSwapChain, "Parents");
	count = container->Create(BufferType::CBV, sizeof(uint32_t), 1, BufferNum::MatchSwapChain, "Count");

	countNum = particleCount;
	count->CopyBuffer(&countNum, sizeof(uint32_t));
}

void Particle::FreeList::Initialize(SHEngine::BufferContainer* container, uint32_t capacityCount) {
	freeList = container->Create(BufferType::SRV_UAV, sizeof(uint32_t), capacityCount, BufferNum::Single, "FreeList");
	freeListCount = container->Create(BufferType::UAV, sizeof(uint32_t), 1, BufferNum::Single, "FreeListCount");
	capacity = container->Create(BufferType::CBV, sizeof(uint32_t), 1, BufferNum::MatchSwapChain, "Capacity");

	this->capacityNum = capacityCount;
	capacity->CopyBuffer(&capacityNum, sizeof(uint32_t));
}
