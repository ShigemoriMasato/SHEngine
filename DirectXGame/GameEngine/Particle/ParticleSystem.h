#pragma once
#include <Particle/System/ParticleProcessor.h>
#include <Particle/System/ParticleRenderer.h>
#include <Particle/Emitter.h>

class ParticleSystem {
public:

	ParticleSystem(uint32_t particleCount, bool isCube = false);

	void Initialize(ICC* icc);
	void Update(ICC* icc);
	void Draw(DCC* dcc);

	// @param name エミッターの名前。ComputeGraphの名前("Particle/<name>/...")に使われる。空なら "Emitter<番号>"
	// @note 同じ名前のエミッターはComputeGraphを共有する(Bindした引数はエミッターごと)
	Particle::Emitter* CreateEmitter(int countNum, ICC* icc, const std::string& name = "");
	void EraseEmitter(Particle::Emitter* emitter);

	void SetCamera(const Camera* camera);

	// ---- 描画設定 ----

	// @brief PixelShaderを差し替える。Shader/以下を指定 (.hlsl まで含める)。デフォルトは "Game/GPUParticle.PS.hlsl"
	// @note 入力は PSInput { float4 position : SV_POSITION; uint instanceID : INSTANCE0; }、出力は SV_TARGET(色) と SV_TARGET1(ブラーマスク)
	void SetPixelShader(const std::string& ps) { renderer_->SetPixelShader(ps); }
	// @brief PixelShaderで使用するCBV/SRV/UAVを追加する。
	// @note SRVの t0 はパーティクルの色 (StructuredBuffer<float16_t4>) で予約済みのため、t1 以降を使うこと
	void SetPSBuffer(BufferType bufferType, int registerNumber, SHEngine::GPUBuffer* buffer) { renderer_->SetPSBuffer(bufferType, registerNumber, buffer); }
	// @brief PixelShaderで使用するCBV/SRV/UAVを、startRegisterから連番で追加する。
	void SetPSBuffers(BufferType bufferType, int startRegister, const std::vector<SHEngine::GPUBuffer*>& buffers) { renderer_->SetPSBuffers(bufferType, startRegister, buffers); }
	// @brief ブレンドモードを設定する。デフォルトは Add
	// @param index 対象のレンダーターゲット番号。負の値なら全てのレンダーターゲット (色とブラーマスク) に設定する
	void SetBlendMode(SHEngine::PSO::BlendStateID blendMode, int index = -1) { renderer_->SetBlendMode(blendMode, index); }
	// @brief PixelShaderでテクスチャ (t8~) を使用するかどうか
	void SetUseTexture(bool useTexture) { renderer_->SetUseTexture(useTexture); }
	// @brief Samplerの設定 (PSO::SamplerID の OR)
	void SetSampler(uint32_t samplerFlag) { renderer_->SetSampler(samplerFlag); }
	void SetSampler(SHEngine::PSO::SamplerID samplerFlag) { renderer_->SetSampler(uint32_t(samplerFlag)); }

	// @brief 描画用のGPUBuffer作成に使えるコンテナ (ParticleSystem と同じ寿命)
	SHEngine::BufferContainer* GetBufferContainer() const { return container_.get(); }

private:

	std::unique_ptr<SHEngine::BufferContainer> container_ = nullptr;

	Particle::Data data_;
	Particle::FreeList freeList_;

	std::unique_ptr<Particle::Processor> processor_;
	std::unique_ptr<Particle::Renderer> renderer_;

	std::vector<std::unique_ptr<Particle::Emitter>> emitters_;

	uint32_t nextEmitterID_ = 0;
};
