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

	indexBuffer_ = std::make_unique<SHEngine::GPUBuffer>(BufferType::CBV, sizeof(int));
	int index = intermediate_->GetTextureData()->GetHandle();
	indexBuffer_->CopyBuffer(&index, sizeof(index));

	maskIndexBuffer_ = std::make_unique<SHEngine::GPUBuffer>(BufferType::CBV, sizeof(int));

	buffer_ = std::make_unique<SHEngine::GPUBuffer>(BufferType::CBV, sizeof(Bloom));

	bloom_.textureWidth = width_;
	bloom_.textureHeight = height_;
}

void BloomEffect::DrawImGui() {
#ifdef USE_IMGUI
	int sampleCount = bloom_.sampleCount;
	if (ImGui::DragInt("Sample Count", &sampleCount, 1)) {
		bloom_.sampleCount = sampleCount;
	}
	ImGui::DragFloat("Strength", &bloom_.strength, 0.01f);

	ImGui::Image(ImTextureRef(intermediate_->GetTextureData()->GetSRVHandle().ptr), ImVec2(320, 180));
#endif
}

void BloomEffect::Save(BinaryManager& bin) const {
	bin.Register(bloom_.sampleCount);
	bin.Register(bloom_.strength);
}

void BloomEffect::Load(BinaryManager& bin) {
	bin.Reverse(bloom_.sampleCount);
	bin.Reverse(bloom_.strength);
}

void BloomEffect::SetBloom(const Bloom& bloom) {
	bloom_.sampleCount = bloom.sampleCount;
	bloom_.strength = bloom.strength;
}

void BloomEffect::Apply(PostEffectContext* context) {
	auto src = context->GetSource();
	auto dst = context->GetDestination();
	auto dcc = context->GetDCC();
	auto renderer = context->GetRenderer();

	// シェーダーでサンプル数で割るため 0 にしない
	bloom_.sampleCount = std::max(bloom_.sampleCount, 1u);

	maskIndexBuffer_->CopyBuffer(&bloom_.maskTexture_, sizeof(bloom_.maskTexture_));
	buffer_->CopyBuffer(&bloom_, sizeof(Bloom));

	// Blurを適用する
	// 横方向のブラー
	dcc->SetRenderTarget(dst);
	renderer->ResetGPUBuffers();
	renderer->SetVS("PostEffect/PostEffect.VS.hlsl");
	renderer->SetPS("PostEffect/BlurHorizontal.PS.hlsl");
	renderer->SetPSBuffers(BufferType::CBV, 0, { maskIndexBuffer_.get(), buffer_.get() });
	renderer->SetUseTexture(true);
	renderer->SetSampler(SHEngine::PSO::SamplerID::ClampClamp_MinMagNearest);
	renderer->Draw(dcc);
	dst->ToTexture(dcc);

	// 縦方向のブラー
	dcc->SetRenderTarget(intermediate_.get());
	renderer->ResetGPUBuffers();
	renderer->SetVS("PostEffect/PostEffect.VS.hlsl");
	renderer->SetPS("PostEffect/BlurVertical.PS.hlsl");
	renderer->SetPSBuffers(BufferType::CBV, 0, { context->GetDestinationIndex(), buffer_.get() });
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
	renderer->SetPSBuffers(BufferType::CBV, 0, { context->GetSourceIndex(), indexBuffer_.get(), buffer_.get() });
	renderer->SetUseTexture(true);
	renderer->SetSampler(SHEngine::PSO::SamplerID::ClampClamp_MinMagNearest);
	renderer->SetBlendState(SHEngine::PSO::BlendStateID::Normal);
	renderer->Draw(dcc);

	dst->ToTexture(dcc);
	context->Toggle();
}

void Bloom::DrawImGui() {
#ifdef USE_IMGUI
	ImGui::DragInt("Sample Count", (int*)&sampleCount, 1, 1, 64);
	ImGui::DragFloat("Strength", &strength, 0.01f, 0.0f, 1.0f);
#endif
}
