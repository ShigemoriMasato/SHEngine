#pragma once
#include "PSOConfig.h"

namespace SHEngine::PSO {

	class ShelfManager {
	public:

		ShelfManager(DXDevice* devive);

		D3D12_SHADER_BYTECODE GetShaderBytecode(ShaderType shaderType, std::string shaderName);
		D3D12_BLEND_DESC GetBlendState(const BlendStateID* id);
		D3D12_DEPTH_STENCIL_DESC GetDepthStencilDesc(DepthStencilID id);
		D3D12_RASTERIZER_DESC GetRasterizerDesc(RasterizerID id);
		ID3D12RootSignature* GetRootSignature(const RootSignatureConfig& config);
		D3D12_INPUT_LAYOUT_DESC GetInputLayoutDesc(InputLayoutID id);

		D3D12_PRIMITIVE_TOPOLOGY_TYPE GetD3D12Topology(Topology id) const;
		D3D_PRIMITIVE_TOPOLOGY GetD3Topology(Topology id) const;

		std::map<SamplerID, D3D12_STATIC_SAMPLER_DESC> GetSamplers() const;

	private:

		/// @brief シェーダー管理
		std::unique_ptr<ShaderShelf> shaderShelf_{};
		/// @brief ブレンドステート管理
		std::unique_ptr<BlendStateShelf> blendStateShelf_{};
		/// @brief 深度ステンシル管理
		std::unique_ptr<DepthStencilShelf> depthStencilShelf_{};
		/// @brief ラスタライザー管理
		std::unique_ptr<RasterizerShelf> rasterizerShelf_{};
		/// @brief ルートシグネチャ管理
		std::unique_ptr<RootSignatureShelf> rootSignatureShelf_{};
		/// @brief 入力レイアウト管理
		std::unique_ptr<InputLayoutShelf> inputLayoutShelf_{};

		std::unordered_map<Topology, D3D12_PRIMITIVE_TOPOLOGY_TYPE> d12topologyMap_;
		std::unordered_map<Topology, D3D_PRIMITIVE_TOPOLOGY> d3topologyMap_;
	};

};
