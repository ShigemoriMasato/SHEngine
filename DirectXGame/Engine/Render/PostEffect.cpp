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

void PostEffect::Initialize(SHEngine::TextureManager* textureManager, SHEngine::DrawData drawData, bool copyOnly) {
	//containerの用意
	container_ = std::make_unique<SHEngine::BufferContainer>(64);

	//使用する
	renderer_ = std::make_unique<SHEngine::Renderer>(drawData);
	renderer_->SetVS("PostEffect/PostEffect.VS.hlsl");
	renderer_->SetSampler(SHEngine::PSO::SamplerID::MagNearest);
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

		//PostEffect用Displayの初期化
		intermediateDisplay_ = std::make_unique<SHEngine::Screen::Display>();
		intermediateDisplay_->Initialize(textureManager, 1280, 720, 0xffffffff);
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
	auto dcc = config.cmdObj;
	auto cmdObject = config.cmdObj->GetCurrentCmdObj();

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
		cmdObject->SetRenderTarget(output);
		origin->ToTexture(cmdObject);
		//bufferにテクスチャをセット
		int textureIndex = origin->GetTextureData()->GetHandle();
		auto& buffer = textureIndexBuffers_.at(drawCount);
		buffer->CopyBuffer(&textureIndex, sizeof(int));
		
		//bufferをset
		renderer_->ResetGPUBuffers();
		renderer_->SetGPUBuffers({ buffer, part.cbvBuffer }, ShaderType::PIXEL_SHADER, BufferType::CBV);

		//描画
		renderer_->Draw(dcc);

		//描画先と描画元の入れ替え
		std::swap(origin, output);
		origin->ToTexture(cmdObject);
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

	cmdObject->SetRenderTarget(output, true);
	origin->ToTexture(cmdObject);
	int textureIndex = origin->GetTextureData()->GetHandle();

	//描画処理
	cmdObject->SetRenderTarget(output);
	origin->ToTexture(cmdObject);
	//bufferにテクスチャをセット
	auto& buffer = textureIndexBuffers_.at(drawCount);
	buffer->CopyBuffer(&textureIndex, sizeof(int));

	//bufferをset
	auto part = parts_.at(PostEffectJob::None);
	renderer_->ResetGPUBuffers();
	renderer_->SetGPUBuffers({ buffer, part.cbvBuffer }, ShaderType::PIXEL_SHADER, BufferType::CBV);

	//描画
	renderer_->Draw(dcc);

	output->ToPresent(cmdObject);
}
