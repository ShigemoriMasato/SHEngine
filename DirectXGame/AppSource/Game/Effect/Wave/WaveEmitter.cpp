#include "WaveEmitter.h"
#include <imgui/imgui.h>
#include <Utility/MatrixFactory.h>
#include <Utility/SearchFile.h>

using namespace Matrix;

constexpr int splitNum = 12;

WaveEmitter::~WaveEmitter() {
}

void WaveEmitter::Initialize(SHEngine::Engine* engine, const Pool& pool) {
	if (kMaxParticleNum_ >= 65535 * 128) {
		throw std::runtime_error("WaveEmitter: maxParticleNum is too large. It must be less than 65535 * 128.");
	}

	container_ = std::make_unique<SHEngine::BufferContainer>();

	auto freeList = container_->Create(BufferType::UAV, sizeof(int), kMaxParticleNum_, BufferNum::Single);
	auto freeListIndex = container_->Create(BufferType::UAV, sizeof(int), 1, BufferNum::Single);
	auto indexList = container_->Create(BufferType::SRV_UAV, sizeof(int), kMaxParticleNum_, BufferNum::Single);
	auto basePositions = container_->Create(BufferType::UAV, sizeof(Vector3) / 2, kMaxParticleNum_, BufferNum::Single); //float16_t3(ローカル座標なので、でかくならないはずと予想)
	auto baseColors = container_->Create(BufferType::UAV, sizeof(Vector3) / 2, kMaxParticleNum_, BufferNum::Single); //float16_t3
	auto velocity = container_->Create(BufferType::UAV, sizeof(Vector3) / 2, kMaxParticleNum_, BufferNum::Single);	// float16_t3
	auto currentTime = container_->Create(BufferType::UAV, sizeof(float) / 2, kMaxParticleNum_, BufferNum::Single); // float16_t

	waveBuffer_ = container_->Create(BufferType::SRV, sizeof(WaveData), kMaxWaveNum_);

	auto maxParticleNum = container_->Create(BufferType::CBV, sizeof(uint32_t), 1, BufferNum::Single);
	updateData_ = container_->Create(BufferType::CBV, sizeof(UpdateData));
	emitData_ = container_->Create(BufferType::CBV, sizeof(EmitData));
	lifeTime_ = container_->Create(BufferType::CBV, sizeof(float));

	maxParticleNum->CopyBuffer(&kMaxParticleNum_, sizeof(uint32_t));

	initialize_ = std::make_unique<SHEngine::ComputeObject>();
	initialize_->SetShader("Particle/Wave/Initialize.CS.hlsl");
	initialize_->SetGPUBuffers(BufferType::UAV, { freeList, freeListIndex, pool.freeList, pool.freeListIndex, indexList, basePositions });
	initialize_->SetGPUBuffers(BufferType::CBV, { maxParticleNum });
	CCC* ccc = engine->GetComputeCommandContext();
	initialize_->SetThreadGroupSize(kMaxParticleNum_ / 1024 + 1);
	initialize_->Execute(ccc);

	emit_ = std::make_unique<SHEngine::ComputeObject>();
	emit_->SetShader("Particle/Wave/Emit.CS.hlsl");
	emit_->SetGPUBuffers(BufferType::UAV, { freeList, freeListIndex, basePositions, baseColors, velocity, currentTime });
	emit_->SetGPUBuffer(BufferType::SRV, indexList);
	emit_->SetGPUBuffers(BufferType::CBV, { emitData_, lifeTime_ });
	emit_->SetUseTexture(true);

	update_ = std::make_unique<SHEngine::ComputeObject>();
	update_->SetShader("Particle/Wave/Update.CS.hlsl");
	update_->SetGPUBuffers(BufferType::UAV, { freeList, freeListIndex, pool.position, pool.color, velocity, currentTime, basePositions, baseColors });
	update_->SetGPUBuffers(BufferType::SRV, { waveBuffer_, indexList });
	update_->SetGPUBuffers(BufferType::CBV, { maxParticleNum, pool.deltaTime, lifeTime_, updateData_ });
	update_->SetThreadGroupSize(kMaxParticleNum_ / 128 + 1);

	waves_.resize(16);
}

void WaveEmitter::Update(CCC* ccc, float deltaTime) {
	emitValue_.seed = GetRandU();
	for (auto& wave : waves_) {
		wave.lifetime += deltaTime;
	}
	CopyConfig(deltaTime);

	emit_->SetThreadGroupSize(config_.emitNum / 128 + 1);
	emit_->Execute(ccc);
	update_->Execute(ccc);
}

void WaveEmitter::AddWave(const WaveData& waveData) {
	//波の情報を追加する
	for (auto& wave : waves_) {
		if (wave.lifetime >= wave.maxlifetime) {
			wave = waveData;
			wave.lifetime = 0.0f;
			return;
		}
	}
}

void WaveEmitter::AddWave(const std::vector<WaveData>& waves) {
	for (const auto& wave : waves) {
		AddWave(wave);
	}
}

void WaveEmitter::DrawImGui() {
#ifdef USE_IMGUI
	ImGui::Begin("WaveEmitter::Lifetime");

	for (int i = 0; i < waves_.size(); ++i) {
		ImGui::Text("Wave %d: %.2f", i, waves_[i].lifetime);
	}

	ImGui::End();
#endif
}

void WaveEmitter::CopyConfig(float deltaTime) {
	emitValue_.speed = config_.speed;
	emitValue_.emitNum = int(float(config_.emitNum) * deltaTime);
	emitValue_.fieldSize = config_.fieldSize;
	emitValue_.color = config_.color;
	emitValue_.textureID = config_.textureID;
	Matrix4x4 parentMatrix = MakeScaleMatrix(config_.scale) * MakeRotationMatrix(config_.rotate) * MakeTranslationMatrix(config_.position);
	updateValue_.parentMatrix = parentMatrix;
	updateValue_.fieldSize = config_.fieldSize;

	lifeTime_->CopyBuffer(&config_.lifeTime, sizeof(float));

	waveBuffer_->CopyBuffer(waves_.data(), sizeof(WaveData) * waves_.size());
	updateData_->CopyBuffer(&updateValue_, sizeof(UpdateData));
	emitData_->CopyBuffer(&emitValue_, sizeof(EmitData));
}

void WaveEmitter::Config::DrawImGui() {
#ifdef USE_IMGUI
	ImGui::Begin("WaveEmitter::Config");

	ImGui::DragFloat("Speed", &speed, 0.01f);
	ImGui::DragFloat("LifeTime", &lifeTime, 0.1f, 5.0f);
	ImGui::DragInt("EmitNum", &emitNum, 1, 0, 100000);
	ImGui::DragFloat("ColorIntensity", &colorIntensity, 0.01f);
	ImGui::ColorEdit3("Color", &color.x);
	ImGui::DragFloat3("FieldSize", &fieldSize.x, 0.1f);
	ImGui::DragFloat3("Scale", &scale.x, 0.1f);
	ImGui::DragFloat3("Rotate", &rotate.x, 0.1f);
	ImGui::DragFloat3("Position", &position.x, 0.1f);

	ImGui::End();

#endif
}

void WaveEmitter::Config::Save(BinaryManager& binaryManager) {
	binaryManager.Register(&color);
	binaryManager.Register(&speed);
	binaryManager.Register(&fieldSize);
	binaryManager.Register(&emitNum);
	binaryManager.Register(&textureID);
	binaryManager.Register(&colorIntensity);
	binaryManager.Register(&lifeTime);
	binaryManager.Register(&scale);
	binaryManager.Register(&rotate);
	binaryManager.Register(&position);
}

void WaveEmitter::Config::Load(BinaryManager& binaryManager) {
	color = binaryManager.Reverse<Vector3>();
	speed = binaryManager.Reverse<float>();
	fieldSize = binaryManager.Reverse<Vector3>();
	emitNum = binaryManager.Reverse<int>();
	textureID = binaryManager.Reverse<uint32_t>();
	colorIntensity = binaryManager.Reverse<float>();
	lifeTime = binaryManager.Reverse<float>();
	scale = binaryManager.Reverse<Vector3>();
	rotate = binaryManager.Reverse<Vector3>();
	position = binaryManager.Reverse<Vector3>();
}

bool WaveEmitter::WaveData::DrawImGui() {
	bool isAdd = false;

#ifdef USE_IMGUI
	ImGui::Begin("WaveEmitter::WaveData");
	
	ImGui::ColorEdit3("TestWave Color", &color.x);
	ImGui::DragFloat3("TestWave Position", &position.x, 0.1f);
	ImGui::DragFloat("TestWave Speed", &speed, 0.01f);
	ImGui::DragFloat("TestWave Intensity", &intensity, 0.01f);
	ImGui::DragFloat("TestWave Lifetime", &lifetime, 0.01f);
	ImGui::DragFloat("TestWave DecayRate", &decayRate, 0.01f);
	ImGui::DragFloat("TestWave MaxLifetime", &maxlifetime, 0.01f);
	ImGui::DragFloat("TestWave Thickness", &thickness, 0.01f);

	if (ImGui::Button("Add")) {
		isAdd = true;
	}

	ImGui::End();
#endif

	return isAdd;
}

void WaveEmitter::WaveData::Save(BinaryManager& binaryManager) {
	binaryManager.Register(&position);
	binaryManager.Register(&speed);
	binaryManager.Register(&color);
	binaryManager.Register(&intensity);
	binaryManager.Register(&lifetime);
	binaryManager.Register(&decayRate);
	binaryManager.Register(&maxlifetime);
	binaryManager.Register(&thickness);
}

void WaveEmitter::WaveData::Load(BinaryManager& binaryManager) {
	position = binaryManager.Reverse<Vector3>();
	speed = binaryManager.Reverse<float>();
	color = binaryManager.Reverse<Vector3>();
	intensity = binaryManager.Reverse<float>();
	lifetime = binaryManager.Reverse<float>();
	decayRate = binaryManager.Reverse<float>();
	maxlifetime = binaryManager.Reverse<float>();
	thickness = binaryManager.Reverse<float>();
}
