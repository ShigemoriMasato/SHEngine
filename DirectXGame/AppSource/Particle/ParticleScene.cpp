#include "ParticleScene.h"

ParticleScene::~ParticleScene() {
}

void ParticleScene::Initialize() {
	container_ = std::make_unique<SHEngine::BufferContainer>();
	camera_ = std::make_unique<DebugCamera>();
	camera_->Initialize(input_);

	particleSystem_.Initialize(computeContext_);
	particleSystem_.SetCamera(camera_.get());


	uint32_t count = 1024 * 1024;
	emitter_ = particleSystem_.CreateEmitter(count, computeContext_);

	SHEngine::ComputeObject* process;
	emitter_->AddTriggerProcess(process);

	discreteBuffer_ = container_->Create(BufferType::CBV, sizeof(float));

	auto textureData = textureManager_->GetTextureData("girl.png");
	auto textureConfig = container_->Create(BufferType::CBV, sizeof(int) * 2);
	auto textureBuffer = container_->Create(textureData);
	auto positionBuffer = container_->Create(BufferType::SRV_UAV, sizeof(Vector3), count, BufferNum::Single);

	auto textureSize = textureData->GetSize();
	int textureConfigData[2] = { (int)textureSize.first, (int)textureSize.second };
	textureConfig->CopyBuffer(textureConfigData, sizeof(int) * 2);

	process->SetShader("Particle/PixelTex/Initialize.CS.hlsl");
	process->SetGPUBuffer(BufferType::CBV, textureConfig);
	process->SetGPUBuffer(BufferType::SRV, textureBuffer);
	process->SetGPUBuffer(BufferType::UAV, positionBuffer);
	process->SetSamplerID(SHEngine::PSO::SamplerID::ClampClamp_MinMagNearest);
	process->SetExecuteNum(count, 128);

	emitter_->AddProcess(process);

	process->SetShader("Particle/PixelTex/Update.CS.hlsl");
	process->SetGPUBuffer(BufferType::CBV, discreteBuffer_);
	process->SetGPUBuffer(BufferType::SRV, positionBuffer);
	process->SetExecuteNum(count, 128);
}

std::unique_ptr<IScene> ParticleScene::Update() {
	camera_->Update();

	// 
	// 更新処理
	//

	particleSystem_.Update(computeContext_);

	return std::unique_ptr<IScene>();
}

void ParticleScene::Draw() {
	auto display = commonData_->display.get();
	auto window = commonData_->window.get();

	directContext_->SetRenderTarget(display);

	auto fence = computeContext_->MiddleExecute();
	directContext_->WaitFenceInGPU(fence);

	particleSystem_.Draw(directContext_);

	fence = directContext_->MiddleExecute();
	computeContext_->WaitFenceInGPU(fence);

	//
	// その他描画処理
	//

	display->ToTexture(directContext_);

	display->DrawImGui();
	imguiManager_.DrawImGui();

#ifdef USE_IMGUI
	ImGui::Begin("System");
	float deltatime = engine_->GetDeltaTime();
	ImGui::Text("FPS: %.2f", 1.0f / deltatime);
	ImGui::End();

	ImGui::Begin("Particle");
	ImGui::DragFloat3("Scale", &transform_.scale.x, 0.01f);
	ImGui::DragFloat3("Rotate", &transform_.rotate.x, 0.01f);
	ImGui::DragFloat3("Position", &transform_.position.x, 0.01f);
	ImGui::DragFloat("Discrete", &discrete_, 1.f, 0.0f);
	ImGui::End();

	emitter_->SetParentMatrix(transform_.Matrix());
	discreteBuffer_->CopyBuffer(&discrete_, sizeof(discrete_));
#endif

	directContext_->SetRenderTarget(window);
	engine_->DrawImGui();
	window->ToPresent(directContext_);
}

void ParticleScene::Save() const {
}

void ParticleScene::Load() {
}
