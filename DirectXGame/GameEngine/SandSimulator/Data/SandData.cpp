#include "SandData.h"

void Sand::Particle::Initialize(SHEngine::BufferContainer* bufferContainer, uint32_t particleCount) {
	position = bufferContainer->Create(BufferType::SRV_UAV, sizeof(float) * 3, particleCount, BufferNum::Single);
	velocity = bufferContainer->Create(BufferType::SRV_UAV, sizeof(float) * 3, particleCount, BufferNum::Single);
	force = bufferContainer->Create(BufferType::SRV_UAV, sizeof(float), particleCount, BufferNum::Single);
	isAlive = bufferContainer->Create(BufferType::SRV_UAV, sizeof(int32_t), particleCount, BufferNum::Single);
	color = bufferContainer->Create(BufferType::CBV, sizeof(float) * 4, 1);
	scale = bufferContainer->Create(BufferType::CBV, sizeof(float), 1);
	count = bufferContainer->Create(BufferType::CBV, sizeof(int32_t), 1);

	count->CopyBuffer(&particleCount, sizeof(int32_t));
	countNum = particleCount;
}

void Sand::FreeList::Initialize(SHEngine::BufferContainer* bufferContainer, uint32_t particleCount) {
	list = bufferContainer->Create(BufferType::SRV_UAV, sizeof(int32_t), particleCount, BufferNum::Single);
	index = bufferContainer->Create(BufferType::CBV, sizeof(int32_t), 1);
	count = bufferContainer->Create(BufferType::CBV, sizeof(int32_t), 1);

	count->CopyBuffer(&particleCount, sizeof(int32_t));
	countNum = particleCount;
}

void Sand::Grid::Initialize(SHEngine::BufferContainer* bufferContainer, uint32_t verticalCount, uint32_t horizontalCount, uint32_t depthCount) {
	gridSize = bufferContainer->Create(BufferType::CBV, sizeof(float));
	gridCount = bufferContainer->Create(BufferType::CBV, sizeof(int32_t) * 3);
	gridStartPos = bufferContainer->Create(BufferType::CBV, sizeof(float) * 3);
	density = bufferContainer->Create(BufferType::SRV_UAV, sizeof(float), verticalCount * horizontalCount * depthCount, BufferNum::Single);

	int gridCountNum[3] = { verticalCount, horizontalCount, depthCount };
	gridCount->CopyBuffer(gridCountNum, sizeof(int32_t));
}
