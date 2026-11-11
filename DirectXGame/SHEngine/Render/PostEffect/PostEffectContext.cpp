#include "PostEffectContext.h"

void PostEffectContext::StaticInitialize(SHEngine::Engine* engine) {
	engine_ = engine;
	vertexPos_ = std::make_unique<SHEngine::GPUBuffer>(BufferType::CBV, sizeof(Vector3), 3);
	vertexUV_ = std::make_unique<SHEngine::GPUBuffer>(BufferType::CBV, sizeof(Vector2), 3);
	std::vector<Vector3> pos = {
		{-1.0f, 1.0f, 0.0f},
		{3.0f, 1.0f, 0.0f},
		{-1.0f, -3.0f, 0.0f},
	};
	std::vector<Vector2> uv = {
		{0.0f, 0.0f},
		{2.0f, 0.0f},
		{0.0f, 2.0f},
	};
	vertexPos_->CopyBuffer(pos.data(), pos.size() * sizeof(Vector3));
	vertexUV_->CopyBuffer(uv.data(), uv.size() * sizeof(Vector2));

	renderer_ = std::make_unique<SHEngine::Renderer>(SHEngine::VertexType::PostEffect);
	renderer_->SetVertexBuffer(SHEngine::VertexType::Position, vertexPos_.get());
	renderer_->SetVertexBuffer(SHEngine::VertexType::Texcoord, vertexUV_.get());

	logger_ = GetLogger("Engine");
}

void PostEffectContext::StaticFinalize() {
	vertexPos_.reset();
	vertexUV_.reset();
	renderer_.reset();
	engine_ = nullptr;
}

void PostEffectContext::Initialize(uint32_t width, uint32_t height) {
	intermediateDisplay_ = std::make_unique<SHEngine::Screen::Display>();
	intermediateDisplay_->Initialize(width, height, "PostEffect::Intermediate");
	intermediateDisplay_->AddRenderTarget(engine_->GetTextureManager(), 0xff);

	intermediateIndexBuffer_ = std::make_unique<SHEngine::GPUBuffer>(BufferType::CBV, sizeof(int));
	int textureIndex = intermediateDisplay_->GetTextureData()->GetHandle();
	intermediateIndexBuffer_->CopyBuffer(&textureIndex, sizeof(int));

	displayIndexBuffer_ = std::make_unique<SHEngine::GPUBuffer>(BufferType::CBV, sizeof(int));
}

void PostEffectContext::Begin(SHEngine::Screen::IDisplay* display) {
	if (display->GetTextureData()->GetSize() != intermediateDisplay_->GetTextureData()->GetSize()) {
		logger_->error("PostEffectContext::Begin: Display size does not match intermediate display size.");
		return;
	}

	display_ = display;
	source_ = display_;
	destination_ = intermediateDisplay_.get();

	int index = display_->GetTextureData()->GetHandle();
	displayIndexBuffer_->CopyBuffer(&index, sizeof(int));
}

void PostEffectContext::End(SHEngine::Screen::IDisplay* output) {
	if (!source_ || !destination_) {
		CommonEnd();
		return;
	}

	//　既にsourceにdisplayが設定されている場合、かつ出力先が指定されていないは入れ替えずに終了する
	if (!output && source_ == display_) {
		CommonEnd();
		return;
	}

	//出力先が指定されていたら切り替える
	if (output) {
		destination_ = output;
	}

	auto dcc = engine_->GetDirectCommandContext();

	dcc->SetRenderTarget(destination_);
	renderer_->ResetGPUBuffers();
	renderer_->SetPS("PostEffect/Copy.PS.hlsl");
	renderer_->SetVS("PostEffect/PostEffect.VS.hlsl");
	renderer_->SetGPUBuffer(GetSourceIndex(), ShaderType::PIXEL_SHADER, BufferType::CBV);
	renderer_->SetUseTexture(true);
	renderer_->SetSampler(SHEngine::PSO::SamplerID::ClampClamp_MinMagNearest);
	renderer_->Draw(dcc);

	CommonEnd();
}

SHEngine::GPUBuffer* PostEffectContext::GetSourceIndex() const {
	if (!display_) {
		return nullptr;
	}

	if (source_ == display_) {
		return displayIndexBuffer_.get();
	}
	return intermediateIndexBuffer_.get();
}

SHEngine::GPUBuffer* PostEffectContext::GetDestinationIndex() const {
	if (!display_) {
		return nullptr;
	}

	if (source_ == display_) {
		return intermediateIndexBuffer_.get();
	}
	return displayIndexBuffer_.get();
}

void PostEffectContext::CommonEnd() {
	display_ = nullptr;
	source_ = nullptr;
	destination_ = nullptr;
}
