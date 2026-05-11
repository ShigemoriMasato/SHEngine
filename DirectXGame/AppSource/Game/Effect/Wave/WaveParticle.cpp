#include "WaveParticle.h"
#include <imgui/imgui.h>
#include <Utility/MatrixFactory.h>

using namespace Matrix;

constexpr int splitNum = 4;

WaveParticle::~WaveParticle() {
	Save();
}

void WaveParticle::Initialize(uint32_t textureID, const Pool& pool, const uint32_t id) {
	container_ = std::make_unique<SHEngine::BufferContainer>();
	auto positions = container_->Create(BufferType::UAV, sizeof(Vector3), pool.maxParticleNum, 1); // position
	auto velocities = container_->Create(BufferType::UAV, sizeof(Vector3), pool.maxParticleNum, 1); // velocity
	auto lifeTimes = container_->Create(BufferType::UAV, sizeof(float), pool.maxParticleNum, 1); // lifeTime
	auto isUse = container_->Create(BufferType::UAV, sizeof(uint32_t), pool.maxParticleNum, 1); // isUse
	waveBuffer_ = container_->Create(BufferType::CBV, sizeof(WaveData) * kMaxWaveNum_, 1); // 波の情報を入れるバッファ
	auto idBuffer = container_->Create(BufferType::CBV, sizeof(uint32_t), 1); // idBuffer
	idBuffer->CopyBuffer(&id, sizeof(uint32_t));

	updateBuffer_.resize(splitNum);
	emitBuffer_.resize(splitNum);
	update_.resize(splitNum);
	emitter_.resize(splitNum);

	for (int i = 0; i < splitNum; ++i) {
		updateBuffer_[i] = container_->Create(BufferType::CBV, sizeof(UpdateData), 1, 1); // UpdateのCBV
		emitBuffer_[i] = container_->Create(BufferType::CBV, sizeof(EmitData), 1, 1); // EmitのCBV

		emitter_[i] = std::make_unique<SHEngine::ComputeObject>("WaveParticle Emitter");
		update_[i] = std::make_unique<SHEngine::ComputeObject>("WaveParticle Update");

		emitter_[i]->SetShader("Particle/Wave/Emit.CS.hlsl");
		emitter_[i]->SetGPUBuffers(BufferType::UAV, { pool.freeList, pool.freeListIndex, pool.type, positions, velocities, lifeTimes, isUse });
		emitter_[i]->SetGPUBuffers(BufferType::CBV, { emitBuffer_[i], idBuffer });
		emitter_[i]->SetUseTexture(true);
		emitter_[i]->SetSamplerID(SHEngine::PSO::SamplerID::ClampClamp_MinMagNearest);

		update_[i]->SetShader("Particle/Wave/Update.CS.hlsl");

		update_[i]->SetGPUBuffers(BufferType::UAV, { pool.freeList, pool.freeListIndex, pool.position, pool.color, pool.type,
			velocities, lifeTimes, positions, isUse });
		update_[i]->SetGPUBuffers(BufferType::CBV, { pool.particleNum, pool.deltaTime, idBuffer, updateBuffer_[i], waveBuffer_ });
		//Updateは全パーティクルを更新する必要があるため、256個ずつで処理をする。
		update_[i]->SetThreadGroupSize(pool.maxParticleNum / 256 / splitNum);
	}

	maxParticleNum_ = pool.maxParticleNum;

	//Loadとかする
	Load();
	emitData_.textureID = textureID;

	//メモリ確保
	waves_.resize(kMaxWaveNum_);
}

void WaveParticle::Update(CmdObj* compute, float deltaTime) {
	emitData_.seed = randomDist_(randomEngine_);
	CopyConfig(deltaTime);

	//Emitterは大量に処理をする必要がないので、64個ずつで処理をする。
	int needThread = int(float(config_.emitNum) * deltaTime / 64.f) + 1;
	for (int i = 0; i < splitNum; ++i) {
		emitter_[i]->SetThreadGroupSize(needThread);
		emitter_[i]->Execute(compute);
	}

	for (size_t i = 0; i < waves_.size(); ++i) {
		auto& wave = waves_[i];
		wave.lifetime += deltaTime;
	}

	waveBuffer_->CopyBuffer(waves_.data(), sizeof(WaveData) * waves_.size());

	for (int i = 0; i < splitNum; ++i) {
		update_[i]->Execute(compute);
	}
}

void WaveParticle::DrawImGui() {
#ifdef USE_IMGUI
	ImGui::Begin("WaveParticle");

	ImGui::DragFloat("Speed", &config_.speed, 0.01f);
	ImGui::DragFloat("LifeTime", &config_.lifeTime, 0.1f, 5.0f);
	ImGui::DragInt("EmitNum", &config_.emitNum, 1, 0, 100000);
	ImGui::DragFloat3("FieldSize", &config_.fieldSize.x, 0.1f);
	ImGui::DragFloat3("Position", &config_.position.x, 0.1f);
	ImGui::DragFloat3("Rotate", &config_.rotate.x, 0.1f);
	ImGui::DragFloat3("Scale", &config_.scale.x, 0.1f);
	ImGui::ColorEdit3("Color", &config_.color.x);

	ImGui::Separator();

	//テスト用の波
	static WaveData testWave = {};
	ImGui::ColorEdit3("TestWave Color", &testWave.color.x);
	ImGui::DragFloat3("TestWave Position", &testWave.position.x, 0.1f);
	ImGui::DragFloat("TestWave Speed", &testWave.speed, 0.01f);
	ImGui::DragFloat("TestWave Intensity", &testWave.intensity, 0.01f);
	ImGui::DragFloat("TestWave Lifetime", &testWave.lifetime, 0.01f);
	ImGui::DragFloat("TestWave DecayRate", &testWave.decayRate, 0.01f);
	ImGui::DragFloat("TestWave MaxLifetime", &testWave.maxlifetime, 0.01f);
	ImGui::DragFloat("TestWave Thickness", &testWave.thickness, 0.01f);

	if (ImGui::Button("Add")) {
		AddWave(testWave);
	}

	ImGui::End();

	ImGui::Begin("Wave Lifetime");
	for (size_t i = 0; i < waves_.size(); ++i) {
		ImGui::Text("Wave %d: %.2f / %.2f", i, waves_[i].lifetime, waves_[i].maxlifetime);
	}
	ImGui::End();

#else

	static WaveData testWave = {};
	static std::vector<Vector3> colorPallete = {
		{1.0f, 0.0f, 0.0f},
		{1.0f, 0.5f, 0.0f},
		{1.0f, 1.0f, 0.0f},
		{0.5f, 1.0f, 0.0f},
		{0.0f, 1.0f, 0.0f},
		{0.0f, 1.0f, 0.5f},
		{0.0f, 1.0f, 1.0f},
		{0.0f, 0.5f, 1.0f},
		{0.0f, 0.0f, 1.0f},
		{0.5f, 0.0f, 1.0f},
		{1.0f, 0.0f, 1.0f},
		{1.0f, 0.0f, 0.5f},
	};
	static int colorIndex = 0;
	colorIndex = (colorIndex) % colorPallete.size();
	testWave.color = colorPallete[colorIndex];
	testWave.speed = 24.0f;
	testWave.intensity = 1.0f;
	testWave.lifetime = 0.0f;
	testWave.decayRate = 0.1f;
	testWave.maxlifetime = 3.0f;
	testWave.thickness = 12.0f;

	static float timer = 0.0f;
	timer += 0.016f;
	if (timer >= 0.5f) {
		colorIndex++;
		timer = 0.0f;
		AddWave(testWave);
	}

#endif
}

void WaveParticle::AddWave(const WaveData& waveData) {
	//波の情報を追加する
	for (auto& wave : waves_) {
		if (wave.lifetime >= wave.maxlifetime) {
			wave = waveData;
			wave.lifetime = 0.0f;
			return;
		}
	}
}

void WaveParticle::Load() {
	if (!binaryManager_.Boot(fileName_)) {
		config_ = {};
		return;
	}

	config_.speed = binaryManager_.Reverse<float>();
	config_.lifeTime = binaryManager_.Reverse<float>();
	config_.emitNum = binaryManager_.Reverse<int>();
	config_.fieldSize = binaryManager_.Reverse<Vector3>();
	config_.position = binaryManager_.Reverse<Vector3>();
	config_.rotate = binaryManager_.Reverse<Vector3>();
	config_.color = binaryManager_.Reverse<Vector3>();
	config_.scale = binaryManager_.Reverse<Vector3>();
}

void WaveParticle::Save() {
	binaryManager_.Boot(fileName_);
	binaryManager_.Register(&config_.speed);
	binaryManager_.Register(&config_.lifeTime);
	binaryManager_.Register(&config_.emitNum);
	binaryManager_.Register(&config_.fieldSize);
	binaryManager_.Register(&config_.position);
	binaryManager_.Register(&config_.rotate);
	binaryManager_.Register(&config_.color);
	binaryManager_.Register(&config_.scale);
	binaryManager_.Write(fileName_);
}

void WaveParticle::CopyConfig(float deltaTime) {
	emitData_.speed = config_.speed;
	emitData_.lifeTime = config_.lifeTime;
	emitData_.emitNum = int(float(config_.emitNum) * deltaTime);
	emitData_.fieldSize = config_.fieldSize;
	Matrix4x4 parentMatrix = MakeScaleMatrix(config_.scale) * MakeRotationMatrix(config_.rotate) * MakeTranslationMatrix(config_.position);
	updateData_.parentMatrix = parentMatrix;
	updateData_.lifeTime = config_.lifeTime;
	updateData_.color = config_.color;
	updateData_.fieldSize = config_.fieldSize;
	for (int i = 0; i < splitNum; ++i) {
		updateData_.executeOffset = i * (maxParticleNum_ / 256 / splitNum);
		emitData_.executeOffset = i * ((int(float(config_.emitNum) * deltaTime / 64.f) + 1) / splitNum);

		updateBuffer_[i]->CopyBuffer(&updateData_, sizeof(UpdateData));
		emitBuffer_[i]->CopyBuffer(&emitData_, sizeof(EmitData));
	}
}
