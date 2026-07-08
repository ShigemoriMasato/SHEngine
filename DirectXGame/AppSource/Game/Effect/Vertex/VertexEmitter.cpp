#include "VertexEmitter.h"

void VertexEmitter::Initialize(SHEngine::Engine* engine, const Pool& pool) {
	positions_ = pool.position;
	colors_ = pool.color;
	freeList_ = pool.freeList;
	freeListIndex_ = pool.freeListIndex;

	addModel_ = std::make_unique<SHEngine::ComputeObject>();
	editColor_ = std::make_unique<SHEngine::ComputeObject>();
	editPosition_ = std::make_unique<SHEngine::ComputeObject>();
	release_ = std::make_unique<SHEngine::ComputeObject>();

	addModel_->SetShader("Particle/Vertex/AddModel.CS.hlsl");
	editColor_->SetShader("Particle/Vertex/EditColor.CS.hlsl");
	editPosition_->SetShader("Particle/Vertex/EditPosition.CS.hlsl");
	release_->SetShader("Particle/Vertex/Release.CS.hlsl");
}

int VertexEmitter::AddModel(const std::vector<Vector3>& vertices, CCC* ccc) {
	Data& data = modelData_.emplace_back();
	data.indexList = container_->Create(BufferType::UAV, sizeof(uint32_t), vertices.size(), BufferNum::Single);
	data.vertexList = container_->Create(BufferType::SRV, sizeof(Vector3), vertices.size());
	data.color = container_->Create(BufferType::CBV, sizeof(Vector3));
	data.vertexNumBuffer = container_->Create(BufferType::CBV, sizeof(uint32_t), 1, BufferNum::Single);		//定数のためSingle
	data.vertexNumBuffer->CopyBuffer(&data.vertexNum, sizeof(uint32_t));

	addModel_->Initialize();
	addModel_->SetGPUBuffers(BufferType::UAV, { freeList_, freeListIndex_, data.indexList, positions_, colors_ });
	addModel_->SetGPUBuffer(BufferType::SRV, data.vertexList);
	addModel_->SetGPUBuffers(BufferType::CBV, { data.vertexNumBuffer, data.color });
	addModel_->SetThreadGroupSize(vertices.size() / 128 + 1);
	addModel_->Execute(ccc);
	return 0;
}

void VertexEmitter::EraseModel(int index, CCC* ccc) {
	Data& data = modelData_[index];

	release_->Initialize();
}

void VertexEmitter::EditColor(int index, const Vector3& color, CCC* ccc) {
}

void VertexEmitter::EditPosition(int index, const std::vector<Vector3>& vertices, CCC* ccc) {
}

void VertexEmitter::Update(CCC* ccc, float deltaTime) {
}
