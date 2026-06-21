#include "DecoObjectRender.h"

using namespace Decorate;

ObjRenderer::ObjRenderer(const SHEngine::DrawData& drawData, int textureIndex) {
	container_ = std::make_unique<SHEngine::BufferContainer>(4);

	//必要なbufferの作成
	cameraBuffer_ = container_->Create(BufferType::CBV, sizeof(Matrix4x4));
	worldBuffer_ = container_->Create(BufferType::SRV, sizeof(Matrix4x4), maxSize_);	//最大100個のインスタンスを想定
	idBuffer_ = container_->Create(BufferType::SRV, sizeof(uint32_t), maxSize_);
	auto textureIndexBuffer = container_->Create(BufferType::CBV, sizeof(int), 1, BufferNum::Single);
	textureIndexBuffer->CopyBuffer(&textureIndex, sizeof(int));

	renderer_ = std::make_unique<SHEngine::Renderer>(drawData);
	renderer_->SetVS("Engine/EditorObj.VS.hlsl");
	renderer_->SetPS("Engine/EditorObj.PS.hlsl");
	renderer_->SetGPUBuffer(cameraBuffer_, ShaderType::VERTEX_SHADER, BufferType::CBV);
	renderer_->SetGPUBuffers({ worldBuffer_, idBuffer_ }, ShaderType::VERTEX_SHADER, BufferType::SRV);
	renderer_->SetGPUBuffer(textureIndexBuffer, ShaderType::PIXEL_SHADER, BufferType::CBV);
	renderer_->SetUseTexture(true);
	renderer_->SetSampler(SHEngine::PSO::SamplerID::Default);
}

void ObjRenderer::Update(Camera* camera) {
	Matrix4x4 vpMatrix = camera->GetVPMatrix();
	cameraBuffer_->CopyBuffer(&vpMatrix, sizeof(Matrix4x4));
}

void ObjRenderer::Draw(DCC* dcc) {
	renderer_->Draw(dcc);
}

void ObjRenderer::SetObjInfo(const std::vector<Matrix4x4>& transforms, const std::vector<uint32_t>& ids) {
	if (uint32_t(transforms.size()) > maxSize_) {
		assert(false && "DecoObject::SetWorldMatrix: Over size.");
		return;
	}
	if (transforms.size() != ids.size()) {
		assert(false && "DecoObject::SetWorldMatrix: transforms and ids size mismatch.");
		return;
	}
	worldBuffer_->CopyBuffer(transforms.data(), sizeof(Matrix4x4) * transforms.size());
	idBuffer_->CopyBuffer(ids.data(), sizeof(uint32_t) * ids.size());
	renderer_->instanceNum_ = uint32_t(transforms.size());
}
