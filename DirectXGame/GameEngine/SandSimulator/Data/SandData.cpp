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

void Sand::Grid::Initialize(SHEngine::BufferContainer* bufferContainer, uint32_t particleCount) {
	GridConfig config{};

	particleIndicesForCell = bufferContainer->Create(BufferType::SRV_UAV, sizeof(uint32_t), particleCount, BufferNum::Single);
	particleCountInCell = bufferContainer->Create(BufferType::SRV_UAV, sizeof(uint32_t), config.cellCount, BufferNum::Single);
	beginIndexForCell = bufferContainer->Create(BufferType::SRV_UAV, sizeof(uint32_t), config.cellCount, BufferNum::Single);
	blockNumber = bufferContainer->Create(BufferType::SRV_UAV, sizeof(uint32_t), config.cellCount, BufferNum::Single);
	gridConfig = bufferContainer->Create(BufferType::CBV, sizeof(GridConfig));

	indexWriteBuffer = bufferContainer->Create(BufferType::UAV, sizeof(uint32_t), config.cellCount, BufferNum::Single);

	config.leftBottomBack = Vector3(-config.width_ * config.cellSize * 0.5f,
		-config.height_ * config.cellSize * 0.5f,
		-config.depth_ * config.cellSize * 0.5f);

	gridConfig->CopyBuffer(&config, sizeof(GridConfig));
}

void Sand::Grid::Update() {
	indexWriteBuffer->ZeroReset();
	particleCountInCell->ZeroReset();
}

void Sand::Grid::SetGridConfig(Vector3 middle) {
	GridConfig config{};
	const float cellSize = config.cellSize;
	config.leftBottomBack = middle - Vector3(config.width_ * cellSize * 0.5f, config.height_ * cellSize * 0.5f, config.depth_ * cellSize * 0.5f);

	// 変数は
	gridConfig->CopyBuffer(&config, sizeof(float) * 3);
}
