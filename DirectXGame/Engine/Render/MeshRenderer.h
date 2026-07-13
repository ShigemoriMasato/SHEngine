#pragma once
#include "Buffer/BufferContainer.h"
#include "DrawDataManager.h"
#include "PSO/PSOEditor.h"
#include "Command/DirectCommandContext.h"
#include "PSO/PSOManagerForMS.h"

namespace SHEngine {

	class MeshRenderer {
	public:

		static void SetPSOEditor(PSO::ManagerMSType* psoEditor, D3D12_GPU_DESCRIPTOR_HANDLE textureStartHandle) { psoEditor_ = psoEditor; textureStartHandle_ = textureStartHandle; }

		MeshRenderer() = default;

		/// @brief GPUBufferをセットする。register順。
		/// @param gpuBuffer 使用するBuffer
		/// @param shaderType どのシェーダーで使用するか
		/// @param bufferType 今回使用するBufferの種類。Buffer作成時に指定したBufferTypeのどれか一つを指定する。複数指定できないことに注意。
		void SetGPUBuffer(GPUBuffer* gpuBuffer, ShaderType shaderType, BufferType bufferType);
		/// @brief GPUBufferをセットする。register順。
		/// @param gpuBuffers GPUBufferの配列
		/// @param shaderType シェーダーの種類
		/// @param bufferType バッファの種類
		void SetGPUBuffers(const std::vector<GPUBuffer*>& gpuBuffers, ShaderType shaderType, BufferType bufferType);
		/// @brief GPUBufferをリセットする。今までSetしたBufferの設定が消える。Buffer自体は解放されない
		void ResetGPUBuffers();
		/// @brief 指定したGPUBufferをリセットする。Buffer自体は解放されない
		/// @param bufferType リセットするBufferをSetしたときに設定したバッファの種類
		/// @param shaderType リセットするBufferをSetしたときに設定したシェーダーの種類
		/// @param gpuBuffer リセットするGPUBuffer
		void EraseGPUBuffer(BufferType bufferType, ShaderType shaderType, GPUBuffer* gpuBuffer);

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

		PSO::ConfigMSType psoConfig_;

		struct BufferConfig {
			BufferType type;
			ShaderType shader;
			GPUBuffer* buffer;
		};
		std::vector<BufferConfig> bufferConfigs_;
		RegisterCounter registerCount_;

		// @brief UAVが含まれているBufferのリスト。描画後、CCCでバリアを張り替えられるようにCommonに変えるために保持する
		std::vector<GPUBuffer*> uavBuffers_;

		// @brief Dispatchするインスタンスの数
		uint32_t groupX_ = 1;
		uint32_t groupY_ = 1;
		uint32_t groupZ_ = 1;

	};

}
