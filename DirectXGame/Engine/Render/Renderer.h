#pragma once
#include "Buffer/BufferContainer.h"
#include "DrawDataManager.h"
#include "PSO/PSOEditor.h"

namespace SHEngine {

	class Renderer {
	public:

		static void SetPSOEditor(PSO::Editor* psoEditor, D3D12_GPU_DESCRIPTOR_HANDLE textureStartHandle) { psoEditor_ = psoEditor; textureStartHandle_ = textureStartHandle; }

		Renderer(const DrawData& drwaData);

		/// <summary>
		/// GPUBufferをセットする。register順。
		/// </summary>
		/// <param name="gpuBuffer"></param>
		/// <param name="type"></param>
		void SetGPUBuffer(GPUBuffer* gpuBuffer, ShaderType shaderType, BufferType bufferType);

		// @brief VertexShaderのファイル名をセットする。デフォルトは"Simple.VS.hlsl"。
		void SetVS(const std::string& vs) { vs_ = vs; }
		// @brief PixelShaderのファイル名をセットする。デフォルトは"White.PS.hlsl"。
		void SetPS(const std::string& ps) { ps_ = ps; }
		// @brief 入力レイアウトIDをセットする。デフォルトはPSO::InputLayoutID::Default。
		void SetInputLayout(PSO::InputLayoutID id) { inputLayoutID_ = id; }
		// @brief ブレンドステートIDをセットする。デフォルトはPSO::BlendStateID::Normal。
		void SetBlendState(PSO::BlendStateID id) { blendID_ = id; }
		// @brief 深度ステンシルIDをセットする。デフォルトはPSO::DepthStencilID::Default。
		void SetDepthStencil(PSO::DepthStencilID id) { depthStencilID_ = id; }
		// @brief ラスタライザーIDをセットする。デフォルトはPSO::RasterizerID::Fill。
		void SetRasterizer(PSO::RasterizerID id) { rasterizerID_ = id; }
		// @brief プリミティブトポロジーをセットする。デフォルトはD3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST。
		void SetTopology(D3D12_PRIMITIVE_TOPOLOGY topology) { topology_ = topology; }
		// @brief スワップチェーン用かどうかをセットする。デフォルトはfalse。
		void SetUseTexture(bool use) { isUseTexture_ = use; }
		// @brief Samplerの設定
		void SetSampler(uint32_t samplerFlag) { samplerFlag_ = samplerFlag; }
		// @brief Samplerの設定
		void SetSampler(PSO::SamplerID samplerFlag) { samplerFlag_ = uint32_t(samplerFlag); }

		// @brief 指定された設定を基に描画コマンドを発行する。
		void Draw(CmdObj* cmdObj);

		// インスタンスの数
		uint32_t instanceNum_ = 1;

	private:

		/// @brief 頂点シェーダーファイル名
		std::string vs_ = "Simple.VS.hlsl";
		/// @brief ピクセルシェーダーファイル名
		std::string ps_ = "White.PS.hlsl";
		/// @brief 入力レイアウトID
		PSO::InputLayoutID inputLayoutID_ = PSO::InputLayoutID::Default;
		/// @brief ブレンドステートID
		PSO::BlendStateID blendID_ = PSO::BlendStateID::Normal;
		/// @brief 深度ステンシルID
		PSO::DepthStencilID depthStencilID_ = PSO::DepthStencilID::Default;
		/// @brief ラスタライザーID
		PSO::RasterizerID rasterizerID_ = PSO::RasterizerID::Fill;
		/// @brief プリミティブトポロジー
		D3D12_PRIMITIVE_TOPOLOGY topology_ = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		/// @brief スワップチェーン用かどうか
		bool isSwapChain_ = false;
		/// @brief 画像を使用するかどうか
		bool isUseTexture_ = false;
		/// @brief Samplerフラグ
		uint32_t samplerFlag_ = uint32_t(PSO::SamplerID::Default);

		static inline PSO::Editor* psoEditor_ = nullptr;
		static inline D3D12_GPU_DESCRIPTOR_HANDLE textureStartHandle_ = {};

		DrawData drawData_;
		std::map<BufferType, std::map<ShaderType, std::vector<GPUBuffer*>>> gpuBuffers_;

	};

}
