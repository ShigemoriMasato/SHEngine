#pragma once
#include "DrawData.h"
#include "Buffer/BufferContainer.h"
#include "PSO/PSOEditor.h"
#include "Command/DirectCommandContext.h"
#include <Camera/Camera.h>

namespace SHEngine {

	class Renderer {
	public:

		static void SetPSOEditor(PSO::Editor* psoEditor, D3D12_GPU_DESCRIPTOR_HANDLE textureStartHandle) { psoEditor_ = psoEditor; textureStartHandle_ = textureStartHandle; }

		Renderer(const ModelData* modelData);

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

		/// @brief GPUBufferをセットする。描画するMesh毎に切り替わる。Meshの数がGPUBufferを超えた場合、最初のBufferに戻る。
		void SetSwitchingGPUBuffer(const std::vector<GPUBuffer*>& gpuBuffers, ShaderType shaderType, BufferType bufferType);

		/// @brief GPUBufferをリセットする。今までSetしたBufferの設定が消える。Buffer自体は解放されない
		void ResetGPUBuffers();

		// @brief 頂点データをSetする。すでにある場合は上書きする
		void SetVertexData(VertexType type, GPUBuffer* gpuBuffer);
		// @brief インデックスデータをSetする。すでにある場合は上書きする
		void SetIndexData(GPUBuffer* gpuBuffer);

		// @brief VertexShaderのファイル名をセットする。デフォルトは"Simple.VS.hlsl"。
		void SetVS(const std::string& vs) { psoConfig_.vs = vs; }
		// @brief PixelShaderのファイル名をセットする。デフォルトは"White.PS.hlsl"。
		void SetPS(const std::string& ps) { psoConfig_.ps = ps; }
		// @brief ブレンドステートIDをセットする。デフォルトはPSO::BlendStateID::Normal。
		void SetBlendState(PSO::BlendStateID id, int index = 0) { psoConfig_.blendID[index] = id; }
		// @brief 深度ステンシルIDをセットする。デフォルトはPSO::DepthStencilID::Default。
		void SetDepthStencil(PSO::DepthStencilID id) { psoConfig_.depthStencilID = id; }
		// @brief ラスタライザーIDをセットする。デフォルトはPSO::RasterizerID::Fill。
		void SetRasterizer(PSO::RasterizerID id) { psoConfig_.rasterizerID = id; }
		// @brief プリミティブトポロジーをセットする。デフォルトはD3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST。
		void SetTopology(PSO::Topology topology) { psoConfig_.topology = topology; }
		// @brief スワップチェーン用かどうかをセットする。デフォルトはfalse。
		void SetUseTexture(bool use) { psoConfig_.rootConfig.useTexture = use; }
		// @brief Samplerの設定
		void SetSampler(uint32_t samplerFlag) { psoConfig_.rootConfig.samplers = samplerFlag; }
		// @brief Samplerの設定
		void SetSampler(PSO::SamplerID samplerFlag) { psoConfig_.rootConfig.samplers = uint32_t(samplerFlag); }

		// @brief 指定された設定を基に描画コマンドを発行する。
		void Draw(DirectCommandContext* dcc);

		// インスタンスの数
		uint32_t instanceNum_ = 1;

	private:

		PSO::Config psoConfig_;

		static inline PSO::Editor* psoEditor_ = nullptr;
		static inline D3D12_GPU_DESCRIPTOR_HANDLE textureStartHandle_ = {};

		std::vector<DrawData> drawData_;

		struct BufferConfig {
			BufferType type;
			ShaderType shader;
			std::vector<GPUBuffer*> buffers;
		};
		std::vector<BufferConfig> bufferConfigs_;
		RegisterCounter registerCount_;

		// @brief UAVが含まれているBufferのリスト。描画後、CCCでバリアを張り替えられるようにCommonに変えるために保持する
		std::vector<GPUBuffer*> uavBuffers_;
	};

}
