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
	uint32_t jobs = config.jobs;
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

void PostEffect::DebugImGui(PostEffectConfig& config, SHEngine::TextureManager* tm, SHEngine::Screen::IDisplay* edgeTexture) {
#ifdef USE_IMGUI
	ImGui::Begin("PostEffect");
	static bool grayScale = false;
	static bool vignette = false;
	static bool boxBlur = false;
	static bool gaussBlur = false;
	static bool edgeDetection = false;
	static bool outline = false;
	static bool radialBlur = false;
	static bool dissolve = false;
	static bool fade = false;
	ImGui::Checkbox("GrayScale", &grayScale);
	if (grayScale) {
		static Grayscale config;
		ImGui::PushID("GrayScale");
		ImGui::DragFloat("intensity", &config.intensity, 0.01f);
		ImGui::PopID();
		CopyBuffer(PostEffectJob::GrayScale, config);
	}
	ImGui::Checkbox("Vignette", &vignette);
	if (vignette) {
		static Vignette config;
		ImGui::PushID("Vignette");
		ImGui::ColorEdit4("Color", &config.color.x);
		ImGui::DragFloat("Strength", &config.intensity, 0.01f);
		ImGui::DragFloat("lerpWidth", &config.radius, 0.01f);
		ImGui::DragFloat("softness", &config.softness, 0.01f);
		ImGui::PopID();
		CopyBuffer(PostEffectJob::Vignette, config);
	}
	ImGui::Checkbox("BoxBlur", &boxBlur);
	if (boxBlur) {
		static Blur config;
		ImGui::PushID("BoxBlur");
		ImGui::SliderInt("KernelSize", reinterpret_cast<int*>(&config.kernelSize), 1, 30);
		ImGui::PopID();
		CopyBuffer(PostEffectJob::BoxBlur, config);
	}
	ImGui::Checkbox("GaussBlur", &gaussBlur);
	if (gaussBlur) {
		static GaussBlur config;
		ImGui::PushID("GaussBlur");
		ImGui::SliderInt("KernelSize", reinterpret_cast<int*>(&config.kernelSize), 1, 15);
		ImGui::DragFloat("Sigma", &config.sigma, 0.01f);
		ImGui::PopID();
		CopyBuffer(PostEffectJob::GaussBlur, config);
	}
	ImGui::Checkbox("EdgeDetection", &edgeDetection);
	if (edgeDetection) {
		//データは必要ないため無記入
	}
	if (edgeTexture) {
		ImGui::Checkbox("Outline", &outline);
		if (outline) {
			static Outline config;
			config.edgeTextureIndex = edgeTexture->GetTextureData()->GetHandle();
			ImGui::PushID("Outline");
			ImGui::ColorEdit4("Color", &config.color.x);
			ImGui::DragFloat("Strength", &config.strength, 0.01f);
			ImGui::PopID();
			CopyBuffer(PostEffectJob::Outline, config);
		}
	}
	ImGui::Checkbox("RadialBlur", &radialBlur);
	if (radialBlur) {
		static RadialBlur config;
		ImGui::PushID("RadialBlur");
		ImGui::DragFloat2("Center", &config.center.x, 0.01f);
		ImGui::DragFloat("Strength", &config.strength, 0.01f);
		ImGui::DragInt("SampleCount", &config.sampleCount, 1, 1, 10);
		ImGui::PopID();
		CopyBuffer(PostEffectJob::RadialBlur, config);
	}
	if (tm) {
		ImGui::Checkbox("Dissolve", &dissolve);
		if (dissolve) {
			static std::vector<int> noise = {
				tm->LoadTexture("Noise0.png"),
				tm->LoadTexture("Noise1.png")
			};
			static Dissolve config;
			static int noiseIndex = 0;
			ImGui::PushID("Dissolve");
			ImGui::SliderInt("NoiseTexture", &noiseIndex, 0, int(noise.size()) - 1);
			ImGui::SliderFloat("Threshold", &config.threshold, 0.0f, 1.0f);
			ImGui::DragFloat("EdgeThreshold", &config.edgeThreshold, 0.01f, 0.0f, 1.0f);
			ImGui::ColorEdit3("EdgeColor", &config.edgeColor.x);
			ImGui::PopID();
			config.noiseTextureIndex = noise[noiseIndex];
			config.transitionTextureIndex = 1;	//トランジションテクスチャのインデックスは1で固定
			CopyBuffer(PostEffectJob::Dissolve, config);
		}
	}
	ImGui::Checkbox("Fade", &fade);
	if (fade) {
		static Fade config;
		ImGui::PushID("Fade");
		ImGui::ColorEdit4("Color", &config.color.x);
		ImGui::DragFloat("intensity", &config.t, 0.01f);
		ImGui::PopID();
		CopyBuffer(PostEffectJob::Fade, config);
	}
	ImGui::End();


	config.jobs =
		uint32_t(grayScale) << 1 |
		uint32_t(vignette) << 2 |
		uint32_t(boxBlur) << 3 |
		uint32_t(gaussBlur) << 4 |
		uint32_t(edgeDetection) << 5 |
		uint32_t(outline) << 6 |
		uint32_t(radialBlur) << 7 |
		uint32_t(dissolve) << 8 |
		uint32_t(fade) << 9;
#endif
}
