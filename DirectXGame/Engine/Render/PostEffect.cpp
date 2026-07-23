#include "PostEffect.h"

uint32_t operator|(PostEffectJob a, PostEffectJob b) {
	return uint32_t(a) | uint32_t(b);
}

uint32_t operator|(uint32_t a, PostEffectJob b) {
	return a | uint32_t(b);
}

uint32_t operator&(uint32_t a, PostEffectJob b) {
	return a & uint32_t(b);
}

uint32_t operator~(PostEffectJob a) {
	return ~uint32_t(a);
}

bool operator<(PostEffectJob a, PostEffectJob b) {
	return uint32_t(a) < uint32_t(b);
}

void PostEffect::StaticInitialize() {
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
}

void PostEffect::StaticFinalize() {
	vertexPos_.reset();
	vertexUV_.reset();
}

void PostEffect::Initialize(SHEngine::TextureManager* textureManager, bool copyOnly) {
	//containerの用意
	container_ = std::make_unique<SHEngine::BufferContainer>(64);

	//使用する
	renderer_ = std::make_unique<SHEngine::Renderer>(SHEngine::VertexType::PostEffect);
	renderer_->SetVertexBuffer(SHEngine::VertexType::Position, vertexPos_.get());
	renderer_->SetVertexBuffer(SHEngine::VertexType::Texcoord, vertexUV_.get());
	renderer_->SetVS("PostEffect/PostEffect.VS.hlsl");
	renderer_->SetSampler(SHEngine::PSO::SamplerID::ClampClamp_MinMagNearest);
	renderer_->SetDepthStencil(SHEngine::PSO::DepthStencilID::UI);
	renderer_->SetUseTexture(true);

	//最後のコピー用に一つ
	int textureIndexBufferNum = 1;

	//RenderObjectの初期化
	auto createPostEffectObject = [this, &textureIndexBufferNum](PostEffectJob job, std::string psPath) {
		Part part;
		part.name = psPath;
		part.cbvBuffer = container_->Create(BufferType::CBV, 512);
		parts_[job] = part;

		//バッファの種類が増えるたび、textureIndexBufferNumを増やす必要がある
		textureIndexBufferNum++;
		};

	createPostEffectObject(PostEffectJob::None, "Simple");

	if (!copyOnly) {
		createPostEffectObject(PostEffectJob::GrayScale, "GrayScale");
		createPostEffectObject(PostEffectJob::Vignette, "Vignette");
		createPostEffectObject(PostEffectJob::BoxBlur, "BoxBlur");
		createPostEffectObject(PostEffectJob::GaussBlur, "GaussBlur");
		createPostEffectObject(PostEffectJob::EdgeDetection, "EdgeDetection");
		createPostEffectObject(PostEffectJob::Outline, "Outline");
		createPostEffectObject(PostEffectJob::RadialBlur, "RadialBlur");
		createPostEffectObject(PostEffectJob::Dissolve, "Dissolve");
		createPostEffectObject(PostEffectJob::Fade, "Fade");

		//PostEffect用Displayの初期化
		intermediateDisplay_ = std::make_unique<SHEngine::Screen::Display>();
		intermediateDisplay_->Initialize(1280, 720, "PostEffect::Intermediate");
		intermediateDisplay_->AddRenderTarget(textureManager, 0xff);
	}

	textureIndexBuffers_.resize(textureIndexBufferNum);
	for (int i = 0; i < textureIndexBufferNum; i++) {
		auto& buffer = textureIndexBuffers_[i];
		buffer = container_->Create(BufferType::CBV, sizeof(int));
	}
}

void PostEffect::Draw(const PostEffectConfig& config) {
	uint32_t jobs = config.jobs_;
	SHEngine::Screen::IDisplay* origin = config.origin;
	SHEngine::Screen::IDisplay* output = intermediateDisplay_.get();
	auto dcc = config.dcc;

	int drawCount = 0;
	for (const auto& [job, part] : parts_) {
		//ジョブがなければ終了
		if (!(jobs & job)) {
			if (jobs == 0) {
				break;
			}
			continue;
		}

		//描画処理
		dcc->SetRenderTarget(output);
		origin->ToTexture(config.dcc);
		//bufferにテクスチャをセット
		int textureIndex = origin->GetTextureData()->GetHandle();
		auto& buffer = textureIndexBuffers_.at(drawCount);
		buffer->CopyBuffer(&textureIndex, sizeof(int));
		
		//bufferをset
		renderer_->ResetGPUBuffers();
		renderer_->SetGPUBuffers({ buffer, part.cbvBuffer }, ShaderType::PIXEL_SHADER, BufferType::CBV);
		renderer_->SetPS("PostEffect/" + part.name + ".PS.hlsl");

		//描画
		renderer_->Draw(dcc);

		//描画先と描画元の入れ替え
		std::swap(origin, output);
		origin->ToTexture(config.dcc);
		//jobを完遂したので削除
		jobs &= ~job;

		drawCount++;
	}

	//最終出力先に描画
	output = config.output;
	//outputがnullptrの場合はoriginに描画
	if (!output) {
		output = config.origin;
	}

	//すでに描画済みなので終了
	if (output == origin) {
		return;
	}

	int textureIndex = origin->GetTextureData()->GetHandle();

	//描画処理
	config.dcc->SetRenderTarget(output);
	origin->ToTexture(config.dcc);
	//bufferにテクスチャをセット
	auto& buffer = textureIndexBuffers_.at(drawCount);
	buffer->CopyBuffer(&textureIndex, sizeof(int));

	//bufferをset
	auto part = parts_.at(PostEffectJob::None);
	renderer_->ResetGPUBuffers();
	renderer_->SetGPUBuffers({ buffer, part.cbvBuffer }, ShaderType::PIXEL_SHADER, BufferType::CBV);
	renderer_->SetPS("PostEffect/" + part.name + ".PS.hlsl");

	//描画
	renderer_->Draw(dcc);

	output->ToPresent(config.dcc);
}
