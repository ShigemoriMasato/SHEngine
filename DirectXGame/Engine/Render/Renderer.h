#pragma once
#include "SHResource.h"
#include "DrawDataManager.h"
#include "PSO/PSOEditor.h"

namespace SHEngine {

	class Renderer {
	public:

		static void SetPSOEditor(PSO::Editor* psoEditor, D3D12_GPU_DESCRIPTOR_HANDLE textureStartHandle) { psoEditor_ = psoEditor; textureStartHandle_ = textureStartHandle; }

		Renderer(DrawData& drwaData);

		/// <summary>
		/// GPUBufferをセットする。register順。
		/// </summary>
		/// <param name="gpuBuffer"></param>
		/// <param name="type"></param>
		void SetGPUBuffer(GPUBuffer* gpuBuffer, ShaderType shaderType, BufferType bufferType);

		void Draw(CmdObj* cmdObj);

		// インスタンスの数
		uint32_t instanceNum_ = 1;

		/// @brief 頂点シェーダーファイル名
		std::string vs_ = "Object3d.VS.hlsl";
		/// @brief ピクセルシェーダーファイル名
		std::string ps_ = "Object3d.PS.hlsl";
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

	private:

		static inline PSO::Editor* psoEditor_ = nullptr;
		static inline D3D12_GPU_DESCRIPTOR_HANDLE textureStartHandle_ = {};

		DrawData drawData_;
		std::map<ShaderType, std::map<BufferType, std::vector<GPUBuffer*>>> gpuBuffers_;

	};

}
