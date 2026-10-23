#include "VertexEmitter.h"

void VertexEmitter::Initialize(SHEngine::Engine* engine, const Pool& pool) {
	positions_ = pool.position;
	colors_ = pool.color;
	freeList_ = pool.freeList;
	freeListIndex_ = pool.freeListIndex;

	addModel_ = std::make_unique<SHEngine::ComputeObject>();
	editColor_ = std::make_unique<SHEngine::ComputeObject>();
	editVertex_ = std::make_unique<SHEngine::ComputeObject>();
	release_ = std::make_unique<SHEngine::ComputeObject>();
	copyBuffer_ = std::make_unique<SHEngine::ComputeObject>();

	addModel_->SetShader("Particle/Vertex/AddModel.CS.hlsl");
	editColor_->SetShader("Particle/Vertex/EditColor.CS.hlsl");
	editVertex_->SetShader("Particle/Vertex/EditVertex.CS.hlsl");
	release_->SetShader("Particle/Vertex/Release.CS.hlsl");
	copyBuffer_->SetShader("Particle/Vertex/CopyBuffer.CS.hlsl");

	container_ = std::make_unique<SHEngine::BufferContainer>();
}

int VertexEmitter::AddModel(const std::vector<Vector3>& vertices, const Vector4& color, CCC* ccc) {
	auto& data = modelData_.emplace_back(std::make_unique<Data>());
	data->indexList = container_->Create(BufferType::SRV_UAV, sizeof(uint32_t), uint32_t(vertices.size()), BufferNum::Single);
	data->vertexList = container_->Create(BufferType::SRV, sizeof(Vector3), uint32_t(vertices.size()));
	data->color = container_->Create(BufferType::CBV, sizeof(Vector4));
	data->vertexNumBuffer = container_->Create(BufferType::CBV, sizeof(uint32_t), 1, BufferNum::Single);		//定数のためSingle
	data->vertexNum = uint32_t(vertices.size());

	data->vertexList->CopyBuffer(vertices.data(), sizeof(Vector3) * uint32_t(vertices.size()));
	data->color->CopyBuffer(&color, sizeof(color));
	data->vertexNumBuffer->CopyBuffer(&data->vertexNum, sizeof(uint32_t));

	addModel_->Initialize();
	addModel_->SetGPUBuffers(BufferType::UAV, { freeList_, freeListIndex_, data->indexList, positions_, colors_ });
	addModel_->SetGPUBuffer(BufferType::SRV, data->vertexList);
	addModel_->SetGPUBuffers(BufferType::CBV, { data->vertexNumBuffer, data->color });
	addModel_->SetExecuteNum(uint32_t(vertices.size()) / 128 + 1);
	addModel_->Execute(ccc);
	return 0;
}

void VertexEmitter::ReleaseModel(int index, CCC* ccc) {
	SizeCheck(index);
	Data* data = modelData_[index].get();

	release_->Initialize();
	release_->SetGPUBuffers(BufferType::UAV, { freeList_, freeListIndex_, data->indexList, positions_, colors_ });
	release_->SetGPUBuffer(BufferType::CBV, data->vertexNumBuffer);
	release_->SetExecuteNum(data->vertexNum / 128 + 1);
	release_->Execute(ccc);

	//メモリを解放する
	container_->Erase(data->indexList);
	container_->Erase(data->vertexList);
	container_->Erase(data->color);
	container_->Erase(data->vertexNumBuffer);
	data->vertexNum = 0;
}

void VertexEmitter::EditColor(int index, const Vector4& color, CCC* ccc) {
	SizeCheck(index);
	Data* data = modelData_[index].get();

	data->color->CopyBuffer(&color, sizeof(color));

	editColor_->Initialize();
	editColor_->SetGPUBuffer(BufferType::UAV, colors_);
	editColor_->SetGPUBuffer(BufferType::SRV, data->indexList);
	editColor_->SetGPUBuffers(BufferType::CBV, { data->vertexNumBuffer, data->color });
	editColor_->SetExecuteNum(data->vertexNum / 128 + 1);
	editColor_->Execute(ccc);
}

void VertexEmitter::EditPosition(int index, const std::vector<Vector3>& vertices, CCC* ccc) {
	SizeCheck(index);
	Data* data = modelData_[index].get();

	data->vertexList->CopyBuffer(vertices.data(), sizeof(Vector3) * uint32_t(vertices.size()));

	editVertex_->Initialize();
	editVertex_->SetGPUBuffer(BufferType::UAV, positions_);
	editVertex_->SetGPUBuffers(BufferType::SRV, { data->indexList, data->vertexList });
	editVertex_->SetGPUBuffers(BufferType::CBV, { data->vertexNumBuffer });
	editVertex_->SetExecuteNum(data->vertexNum / 128 + 1);
	editVertex_->Execute(ccc);
}

void VertexEmitter::CopyIndexList(int index, SHEngine::ReadBackBuffer* dest, CCC* ccc) {
	SizeCheck(index);
	Data* data = modelData_[index].get();
	dest->Copy(data->indexList, ccc);
}

void VertexEmitter::Update(CCC* ccc, float deltaTime) {
}

void VertexEmitter::SizeCheck(int index) {
	if (index < 0 || index >= modelData_.size()) {
		throw std::out_of_range("VertexEmitter::SizeCheck: index out of range");
	}
	if (modelData_[index]->vertexNumBuffer == nullptr) {
		throw std::runtime_error("VertexEmitter::SizeCheck: Already Released");
	}
}
