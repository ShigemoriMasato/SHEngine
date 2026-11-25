#pragma once
#include "Buffer/BufferContainer.h"
#include "DrawData.h"
#include "PSO/PSOEditor.h"
#include "Command/DirectCommandContext.h"
#include "PSO/PSOManagerForMS.h"

namespace SHEngine {

	class MeshRenderer {
	public:

		static void SetPSOEditor(PSO::ManagerMSType* psoEditor, D3D12_GPU_DESCRIPTOR_HANDLE textureStartHandle) { psoEditor_ = psoEditor; textureStartHandle_ = textureStartHandle; }

		MeshRenderer() = default;

		/// @brief AmplificationShaderで使用するGPUBufferをセットする。セットした順番がRootParameterIndexになる。
		/// @param bufferType 今回使用するBufferの種類。Buffer作成時に指定したBufferTypeのどれか一つを指定する。複数指定できないことに注意。
		/// @param registerNumber シェーダー側のレジスタ番号(b0ならCBVの0)
		/// @param gpuBuffer 使用するBuffer
		/// @note 同じbufferType・registerNumberが既にセットされている場合はBufferを差し替える。
		void SetASBuffer(BufferType bufferType, int registerNumber, GPUBuffer* gpuBuffer);
		/// @brief AmplificationShaderで使用するGPUBufferを、startRegisterから連番でセットする。
		void SetASBuffers(BufferType bufferType, int startRegister, const std::vector<GPUBuffer*>& gpuBuffers);
		/// @brief AmplificationShaderにセットしたGPUBufferを外す。Buffer自体は解放されない
		void EraseASBuffer(BufferType bufferType, int registerNumber);

		/// @brief MeshShaderで使用するGPUBufferをセットする。セットした順番がRootParameterIndexになる。
		/// @param bufferType 今回使用するBufferの種類。Buffer作成時に指定したBufferTypeのどれか一つを指定する。複数指定できないことに注意。
		/// @param registerNumber シェーダー側のレジスタ番号(b0ならCBVの0)
		/// @param gpuBuffer 使用するBuffer
		/// @note 同じbufferType・registerNumberが既にセットされている場合はBufferを差し替える。
		void SetMSBuffer(BufferType bufferType, int registerNumber, GPUBuffer* gpuBuffer);
		/// @brief MeshShaderで使用するGPUBufferを、startRegisterから連番でセットする。
		void SetMSBuffers(BufferType bufferType, int startRegister, const std::vector<GPUBuffer*>& gpuBuffers);
		/// @brief MeshShaderにセットしたGPUBufferを外す。Buffer自体は解放されない
		void EraseMSBuffer(BufferType bufferType, int registerNumber);

		/// @brief PixelShaderで使用するGPUBufferをセットする。セットした順番がRootParameterIndexになる。
		/// @param bufferType 今回使用するBufferの種類。Buffer作成時に指定したBufferTypeのどれか一つを指定する。複数指定できないことに注意。
		/// @param registerNumber シェーダー側のレジスタ番号(b0ならCBVの0)
		/// @param gpuBuffer 使用するBuffer
		/// @note 同じbufferType・registerNumberが既にセットされている場合はBufferを差し替える。
		void SetPSBuffer(BufferType bufferType, int registerNumber, GPUBuffer* gpuBuffer);
		/// @brief PixelShaderで使用するGPUBufferを、startRegisterから連番でセットする。
		void SetPSBuffers(BufferType bufferType, int startRegister, const std::vector<GPUBuffer*>& gpuBuffers);
		/// @brief PixelShaderにセットしたGPUBufferを外す。Buffer自体は解放されない
		void ErasePSBuffer(BufferType bufferType, int registerNumber);

		/// @brief GPUBufferをリセットする。今までSetしたBufferの設定が消える。Buffer自体は解放されない
		void ResetGPUBuffers();

		// @brief AmplificationShaderのファイル名をセットする。デフォルトは"Simple.AS.hlsl"。
		void SetAS(const std::string& as) { psoConfig_.as = as; }
		// @brief VertexShaderのファイル名をセットする。デフォルトは"Simple.VS.hlsl"。
		void SetMS(const std::string& ms) { psoConfig_.ms = ms; }
		// @brief PixelShaderのファイル名をセットする。デフォルトは"White.PS.hlsl"。
		void SetPS(const std::string& ps) { psoConfig_.ps = ps; }
		// @brief ブレンドステートIDをセットする。デフォルトはPSO::BlendStateID::Normal。
		void SetBlendState(PSO::BlendStateID id, int index = 0) { psoConfig_.blendID[index] = id; }
		// @brief 深度ステンシルIDをセットする。デフォルトはPSO::DepthStencilID::Default。
		void SetDepthStencil(PSO::DepthStencilID id) { psoConfig_.depthStencilID = id; }
		// @brief ラスタライザーIDをセットする。デフォルトはPSO::RasterizerID::Fill。
		void SetRasterizer(PSO::RasterizerID id) { psoConfig_.rasterizerID = id; }
		// @brief スワップチェーン用かどうかをセットする。デフォルトはfalse。
		void SetUseTexture(bool use) { psoConfig_.rootConfig.useTexture = use; }
		// @brief Samplerの設定
		void SetSampler(uint32_t samplerFlag) { psoConfig_.rootConfig.samplers = samplerFlag; }
		// @brief Samplerの設定
		void SetSampler(PSO::SamplerID samplerFlag) { psoConfig_.rootConfig.samplers = uint32_t(samplerFlag); }

		// @brief Dispatchするインスタンスの数をセットする。デフォルトは1。
		void SetDispatchGroup(uint32_t groupX, uint32_t groupY = 1, uint32_t groupZ = 1) { groupX_ = groupX; groupY_ = groupY; groupZ_ = groupZ; }

		// @brief 指定された設定を基に描画コマンドを発行する。
		void Draw(DirectCommandContext* dcc);

	private:

		static inline PSO::ManagerMSType* psoEditor_ = nullptr;
		static inline D3D12_GPU_DESCRIPTOR_HANDLE textureStartHandle_ = {};
		static inline Logger logger_ = GetLogger("Engine");

		// @brief Set**Bufferの共通処理
		void SetGPUBuffer(ShaderType shaderType, BufferType bufferType, int registerNumber, GPUBuffer* gpuBuffer);
		// @brief Erase**Bufferの共通処理
		void EraseGPUBuffer(ShaderType shaderType, BufferType bufferType, int registerNumber);
		// @brief Set**Buffersの共通処理
		void SetGPUBuffers(ShaderType shaderType, BufferType bufferType, int startRegister, const std::vector<GPUBuffer*>& gpuBuffers);

		PSO::ConfigMSType psoConfig_;

		struct BufferConfig {
			BufferType type;
			ShaderType shader;
			GPUBuffer* buffer;
		};
		// psoConfig_.rootConfig.rootParamsと同じ順番で保持する
		std::vector<BufferConfig> bufferConfigs_;

		// @brief UAVが含まれているBufferのリスト。描画後、CCCでバリアを張り替えられるようにCommonに変えるために保持する
		std::vector<GPUBuffer*> uavBuffers_;

		// @brief Dispatchするインスタンスの数
		uint32_t groupX_ = 1;
		uint32_t groupY_ = 1;
		uint32_t groupZ_ = 1;

	};

}
