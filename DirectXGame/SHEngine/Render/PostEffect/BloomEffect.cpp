#include "BloomEffect.h"
#include <algorithm>
#include <array>
#include <string>
#include <imgui/imgui.h>

void BloomEffect::Initialize(SHEngine::Engine& engine) {
	auto tm = engine.GetTextureManager();
	intermediate_ = std::make_unique<SHEngine::Screen::Display>();
	intermediate_->Initialize(width_, height_, "Bloom::Intermediate");
	intermediate_->AddRenderTarget(tm, 0xff);

	composite_ = std::make_unique<SHEngine::Screen::Display>();
	composite_->Initialize(width_, height_ * 2, "Bloom::Composite");
	composite_->AddRenderTarget(tm, 0xff);

	indexBuffer_ = std::make_unique<SHEngine::GPUBuffer>(BufferType::CBV, sizeof(int));
	int index = intermediate_->GetTextureData()->GetHandle();
	indexBuffer_->CopyBuffer(&index, sizeof(index));

	compositeIndexBuffer_ = std::make_unique<SHEngine::GPUBuffer>(BufferType::CBV, sizeof(int));
	index = composite_->GetTextureData()->GetHandle();
	compositeIndexBuffer_->CopyBuffer(&index, sizeof(index));

	maskIndexBuffer_ = std::make_unique<SHEngine::GPUBuffer>(BufferType::CBV, sizeof(int));

	buffer_ = std::make_unique<SHEngine::GPUBuffer>(BufferType::CBV, sizeof(Bloom));
}

void BloomEffect::DrawImGui() {
#ifdef USE_IMGUI
	static int clientKernel = bloom_.clientKernel;
	if (ImGui::DragInt("Client Kernel", &clientKernel, 1)) {
		bloom_.clientKernel = std::max(1, clientKernel);
	}

	ImGui::DragFloat("Sigma", &bloom_.sigma, 0.01f);
	ImGui::DragFloat("Strength", &bloom_.strength, 0.01f);

	static int iterations = bloom_.iterations;
	if (ImGui::DragInt("Iterations", &iterations, 1, 1, 8)) {
		bloom_.iterations = iterations;
	}

	ImGui::Image(ImTextureRef(intermediate_->GetTextureData()->GetSRVHandle().ptr), ImVec2(320, 180));
#endif
}

void BloomEffect::Save(BinaryManager& bin) const {
	bin.Register(bloom_.clientKernel);
	bin.Register(bloom_.sigma);
	bin.Register(bloom_.strength);
	bin.Register(bloom_.iterations);
}

void BloomEffect::Load(BinaryManager& bin) {
	bin.Reverse(bloom_.clientKernel);
	bin.Reverse(bloom_.sigma);
	bin.Reverse(bloom_.strength);
	bin.Reverse(bloom_.iterations);
	SetBloom(bloom_);
}

void BloomEffect::SetBloom(const Bloom& bloom) {
	bloom_ = bloom;
}

void BloomEffect::Apply(PostEffectContext* context) {
	auto src = context->GetSource();
	auto dst = context->GetDestination();
	auto dcc = context->GetDCC();
	auto renderer = context->GetRenderer();

	maskIndexBuffer_->CopyBuffer(&bloom_.maskTexture_, sizeof(bloom_.maskTexture_));
	buffer_->CopyBuffer(&bloom_, sizeof(Bloom));

	renderer->ResetGPUBuffers();
	renderer->SetVS("PostEffect/PostEffect.VS.hlsl");
	renderer->SetPS("PostEffect/GaussBlur.PS.hlsl");
	renderer->SetGPUBuffers({ maskIndexBuffer_.get(), buffer_.get() }, ShaderType::PIXEL_SHADER, BufferType::CBV);
	renderer->SetUseTexture(true);
	renderer->SetSampler(SHEngine::PSO::SamplerID::ClampClamp_MinMagNearest);

	float drawY = 0.0f;
	float scale = 1.0f;

	// 中間レンダーターゲットに対して、縮小した画像を描画していく
	dcc->SetRenderTarget(composite_.get(), true, false);
	for (uint32_t i = 0; i < bloom_.iterations; ++i) {
		Vector2 min = { 0.0f, drawY };
		Vector2 size = Vector2(static_cast<float>(width_), static_cast<float>(height_)) * scale;

		dst->SetViewport(dcc, min, size);
		drawY += size.y;
		scale *= 0.5f;

		renderer->Draw(dcc);
	}

	composite_->ToTexture(dcc);

	// 中間レンダーターゲットのテクスチャを引き伸ばし、平均をとるように合成する
	dcc->SetRenderTarget(intermediate_.get());
	renderer->ResetGPUBuffers();
	renderer->SetVS("PostEffect/PostEffect.VS.hlsl");
	renderer->SetPS("PostEffect/BloomComposite.PS.hlsl");
	renderer->SetGPUBuffers({ compositeIndexBuffer_.get(), buffer_.get()}, ShaderType::PIXEL_SHADER, BufferType::CBV);
	renderer->SetUseTexture(true);
	renderer->SetSampler(SHEngine::PSO::SamplerID::ClampClamp_MinMagNearest);
	renderer->Draw(dcc);

	intermediate_->ToTexture(dcc);
	src->ToTexture(dcc);

	// 平均テクスチャと元のテクスチャを加算合成する
	dcc->SetRenderTarget(dst);
	renderer->ResetGPUBuffers();
	renderer->SetVS("PostEffect/PostEffect.VS.hlsl");
	renderer->SetPS("PostEffect/Bloom.PS.hlsl");
	renderer->SetGPUBuffers({ context->GetSourceIndex(), indexBuffer_.get(), buffer_.get() }, ShaderType::PIXEL_SHADER, BufferType::CBV);
	renderer->SetUseTexture(true);
	renderer->SetSampler(SHEngine::PSO::SamplerID::ClampClamp_MinMagNearest);
	renderer->Draw(dcc);

	dst->ToTexture(dcc);
	context->Toggle();
}
