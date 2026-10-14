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

void Sand::Grid::Initialize(SHEngine::BufferContainer* bufferContainer, float cellSize) {
	GridConfig config{};
	cellIndices = bufferContainer->Create(BufferType::SRV_UAV, sizeof(int32_t) * config.particlePerCell_, config.cellCount, BufferNum::Single);
	cellCounts = bufferContainer->Create(BufferType::SRV_UAV, sizeof(int32_t), config.cellCount, BufferNum::Single);
	gridConfig = bufferContainer->Create(BufferType::CBV, sizeof(GridConfig), 1);

	config.gridOrigin = -Vector3(config.width_ * cellSize * 0.5f, config.height_ * cellSize * 0.5f, config.depth_ * cellSize * 0.5f);
	config.cellSize = cellSize;
	gridConfig->CopyBuffer(&config, sizeof(GridConfig));
}

void Sand::Grid::SetGridConfig(Vector3 middle, float cellSize) {
	GridConfig config{};
	config.gridOrigin = middle - Vector3(config.width_ * cellSize * 0.5f, config.height_ * cellSize * 0.5f, config.depth_ * cellSize * 0.5f);
	config.cellSize = cellSize;

	//originとcellsize以外は定数なので、最初の四つ(float4)だけコピーする
	gridConfig->CopyBuffer(&config, sizeof(float) * 4);
}
