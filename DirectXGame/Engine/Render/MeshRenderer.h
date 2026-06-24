#pragma once
#include "Buffer/BufferContainer.h"
#include "DrawDataManager.h"
#include "PSO/PSOEditor.h"
#include "Command/DirectCommandContext.h"

namespace SHEngine {

	class MeshRenderer {

		static void SetPSOEditor(PSO::Editor* psoEditor, D3D12_GPU_DESCRIPTOR_HANDLE textureStartHandle) { psoEditor_ = psoEditor; textureStartHandle_ = textureStartHandle; }

		MeshRenderer(const DrawData& drawData);

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
		void SetMS(const std::string& ms) { ms_ = ms; }
		// @brief PixelShaderのファイル名をセットする。デフォルトは"White.PS.hlsl"。
		void SetPS(const std::string& ps) { ps_ = ps; }
		// @brief ブレンドステートIDをセットする。デフォルトはPSO::BlendStateID::Normal。
		void SetBlendState(PSO::BlendStateID id, int index = 0) { blendID_[index] = id; }
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
		void Draw(DirectCommandContext* dcc);

		// インスタンスの数
		uint32_t instanceNum_ = 1;

	private:

		/// @brief 頂点シェーダーファイル名
		std::string ms_ = "Simple.MS.hlsl";
		/// @brief ピクセルシェーダーファイル名
		std::string ps_ = "White.PS.hlsl";
		/// @brief ブレンドステートID
		PSO::BlendStateID blendID_[8];
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
