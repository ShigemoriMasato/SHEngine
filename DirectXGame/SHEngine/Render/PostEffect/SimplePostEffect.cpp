#include "SimplePostEffect.h"

SimplePostEffect::SimplePostEffect(std::unique_ptr<ISPEData> data, size_t size, const std::string& shaderName) {
	data_ = std::move(data);
	size_ = size;
	shaderPath_ = "PostEffect/" + shaderName + ".PS.hlsl";
	buffer_ = std::make_unique<SHEngine::GPUBuffer>(BufferType::CBV, size_);
}

void SimplePostEffect::Initialize(SHEngine::Engine& engine) {

}

void SimplePostEffect::DrawImGui() {
	data_->DrawImGui();
	buffer_->CopyBuffer(data_.get(), size_);
}

void SimplePostEffect::Save(BinaryManager& bin) const {
	data_->Save(bin);
}

void SimplePostEffect::Load(BinaryManager& bin) {
	data_->Load(bin);
	buffer_->CopyBuffer(data_.get(), size_);
}

void SimplePostEffect::Apply(PostEffectContext* context) {
	auto dcc = context->GetDCC();
	auto dest = context->GetDestination();
	auto src = context->GetSource();
	auto srcIndex = context->GetSourceIndex();

	src->ToTexture(dcc);

	auto renderer = context->GetRenderer();
	renderer->ResetGPUBuffers();
	renderer->SetVS("PostEffect/PostEffect.VS.hlsl");
	renderer->SetPS(shaderPath_);
	renderer->SetGPUBuffers({ srcIndex, buffer_.get() }, ShaderType::PIXEL_SHADER, BufferType::CBV);
	renderer->SetUseTexture(true);
	renderer->SetSampler(SHEngine::PSO::SamplerID::ClampClamp_MinMagNearest);

	dcc->SetRenderTarget(dest);
	renderer->Draw(dcc);

	dest->ToTexture(dcc);

	context->Toggle();
}
