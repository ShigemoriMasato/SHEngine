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

		Renderer(VertexType type, const Mesh& modelData);
		/// @brief 頂点情報の種類を指定してRendererを作成する。頂点情報は自前で作る必要がある。
		Renderer(VertexType type);

		/// @brief 頂点情報に変化がある場合に使用する。自前で作ったリソースをVertexBufferとしてセットする。
		void SetVertexBuffer(VertexType type, GPUBuffer* gpuBuffer);
		/// @brief インデックス情報に変化がある場合に使用する。自前で作ったリソースをIndexBufferとしてセットする。
		void SetIndexBuffer(GPUBuffer* gpuBuffer);

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
		static inline Logger logger_ = GetLogger("Engine");

		std::unique_ptr<BufferContainer> container_;
		std::vector<GPUBuffer*> vertexBuffers_;
		GPUBuffer* indexBuffer_;
		DrawData drawData_;
		VertexType vertexType_;

		struct BufferConfig {
			BufferType type;
			ShaderType shader;
			GPUBuffer* buffer;
		};
		std::vector<BufferConfig> bufferConfigs_;
		RegisterCounter registerCount_;

		// @brief UAVが含まれているBufferのリスト。描画後、CCCでバリアを張り替えられるようにCommonに変えるために保持する
		std::vector<GPUBuffer*> uavBuffers_;
	};

}
