#include "SandSimulator.h"

SandSimulator::SandSimulator(const Sand::InitializeData& initData) {
	container_ = std::make_unique<SHEngine::BufferContainer>();
	particleData_.Initialize(container_.get(), initData.particleCount);
	gridData_.Initialize(container_.get(), initData.gridVerticalCount, initData.gridHorizontalCount, initData.gridDepthCount);
	freeList_.Initialize(container_.get(), initData.particleCount);
}

void SandSimulator::Initialize(const SHEngine::ICommandContext* commandContext) {
}

void SandSimulator::Update(float deltaTime, const SHEngine::ICommandContext* commandContext) {
}

void SandSimulator::Draw(const DCC* dcc) {
}

void SandSimulator::DebugDraw(const DCC* dcc) {
}

void SandSimulator::AddSand(const Sand::AddSandConfig& config) {
}
