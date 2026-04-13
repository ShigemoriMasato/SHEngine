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

void PostEffect::Initialize(SHEngine::TextureManager* textureManager, SHEngine::DrawData drawData) {
	//PostEffect用Displayの初期化
	intermediateDisplay_ = std::make_unique<SHEngine::Screen::MultiDisplay>();
	intermediateDisplay_->Initialize(1280, 720, 0xffffffff, textureManager);

	//RenderObjectの初期化
	auto createPostEffectObject = [&](PostEffectJob job, std::string psPath) {
		auto postEffectObject = std::make_unique<SHEngine::RenderObject>("PostEffect::" + psPath);
		postEffectObject->Initialize();
		postEffectObject->psoConfig_.vs = "PostEffect/PostEffect.VS.hlsl";
		postEffectObject->psoConfig_.ps = "PostEffect/" + psPath + ".PS.hlsl";
		postEffectObject->psoConfig_.rootConfig.samplers = uint32_t(SHEngine::PSO::SamplerID::ClampClamp_MinMagNearest);
		postEffectObject->SetUseTexture(true);
		postEffectObject->SetDrawData(drawData);
		postEffectObject->CreateCBV(sizeof(int), ShaderType::PIXEL_SHADER, "PostEffect::SourceTexture");
		postEffectObject->CreateCBV(512, ShaderType::PIXEL_SHADER, "PostEffect::SourceTextureOffset");
		postEffectObjects_[job] = std::move(postEffectObject);
		};

	createPostEffectObject(PostEffectJob::None, "Simple");
	createPostEffectObject(PostEffectJob::BlurV, "BlurVert");
	createPostEffectObject(PostEffectJob::BlurH, "BlurHori");
	createPostEffectObject(PostEffectJob::Fade, "Fade");
	createPostEffectObject(PostEffectJob::Glitch, "Glitch");
	createPostEffectObject(PostEffectJob::GrayScale, "GrayScale");
	createPostEffectObject(PostEffectJob::GridTransition, "GridTransition");
	createPostEffectObject(PostEffectJob::SlowMotion, "SlowMotion");
	createPostEffectObject(PostEffectJob::HeavyBlur, "Blur");
}

void PostEffect::Draw(const PostEffectConfig& config) {
	uint32_t jobs = config.jobs_;
	SHEngine::Screen::IDisplay* origin = config.origin;
	SHEngine::Screen::IDisplay* output = intermediateDisplay_.get();
	auto cmdObject = config.cmdObj;

	//jobがなければ飛ばす
	if (jobs == 0) {
		goto FINAL_DRAW;
	}

	for (const auto& [job, obj] : postEffectObjects_) {
		//ジョブがなければ終了
		if (!(jobs & job)) {
			continue;
		}

		//描画処理
		output->PreDraw(cmdObject, true);
		origin->ToTexture(cmdObject);
		//RenderObjectにテクスチャをセット
		int textureIndex = origin->GetTextureData()->GetOffset();
		obj->CopyBufferData(0, &textureIndex, sizeof(int));
		obj->psoConfig_.isSwapChain = output->GetRTVFormat() == DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		obj->Draw(cmdObject);

		//描画先と描画元の入れ替え
		std::swap(origin, output);
		origin->ToTexture(cmdObject);
		//jobを完遂したので削除
		jobs &= ~job;

		if(jobs == 0) {
			break;
		}
	}

FINAL_DRAW:

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

	output->PreDraw(cmdObject, false);
	origin->ToTexture(cmdObject);
	int textureIndex = origin->GetTextureData()->GetOffset();
	auto finalObj = postEffectObjects_.at(PostEffectJob::None).get();
	finalObj->CopyBufferData(0, &textureIndex, sizeof(int));
	finalObj->psoConfig_.isSwapChain = output->GetRTVFormat() == DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	finalObj->Draw(cmdObject);
	output->PostDraw(cmdObject);
}
