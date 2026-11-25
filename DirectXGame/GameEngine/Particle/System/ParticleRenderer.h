#pragma once
#include <Particle/System/ParticleData.h>
#include <Render/MeshRenderer.h>
#include <Camera/Camera.h>

namespace Particle {

	class Renderer {
	public:

		void Initialize(Data* data, SHEngine::BufferContainer* container, bool isCube);
		void Update();
		void Draw(DCC* dcc);

		void SetInstance(uint32_t instanceNum);
		void SetCamera(const Camera* camera);

		// @brief PixelShaderを差し替える。Shader/以下を指定 (.hlsl まで含める)。デフォルトは "Game/GPUParticle.PS.hlsl"
		void SetPixelShader(const std::string& ps);
		// @brief PixelShaderで使用するCBV/SRV/UAVを追加する。
		// @param bufferType 今回使用するBufferの種類(CBV/SRV/UAVのどれか一つ)
		// @param registerNumber シェーダー側のレジスタ番号
		// @param buffer 使用するGPUBuffer
		// @note SRVの t0 はパーティクルの色 (StructuredBuffer<float16_t4>) で予約済みのため、t1 以降を使うこと
		void SetPSBuffer(BufferType bufferType, int registerNumber, SHEngine::GPUBuffer* buffer);
		// @brief PixelShaderで使用するCBV/SRV/UAVを、startRegisterから連番で追加する。
		void SetPSBuffers(BufferType bufferType, int startRegister, const std::vector<SHEngine::GPUBuffer*>& buffers);
		// @brief ブレンドモードを設定する
		// @param index 対象のレンダーターゲット番号。負の値なら全てのレンダーターゲット (色とブラーマスク) に設定する
		void SetBlendMode(SHEngine::PSO::BlendStateID blendMode, int index = -1);
		// @brief PixelShaderでテクスチャ (t8~) を使用するかどうか
		void SetUseTexture(bool useTexture);
		// @brief Samplerの設定 (PSO::SamplerID の OR)
		void SetSampler(uint32_t samplerFlag);

	private:

		// レンダーターゲット数 (SV_TARGET: 色, SV_TARGET1: ブラーマスク)
		static constexpr int kRenderTargetCount = 2;

		const Camera* camera_;

		struct CameraData {
			Matrix4x4 vpMatrix;
			Matrix4x4 billboardMatrix;
		} cameraData_;
		SHEngine::GPUBuffer* cameraBuffer_ = nullptr;

		std::unique_ptr<SHEngine::MeshRenderer> meshRenderer_;

	};

}
