#pragma once
#include "PSOConfig.h"
#include "ShelfManager.h"
#include <DirectXTex/d3dx12.h>

namespace SHEngine::PSO {

	struct ConfigMSType {
		std::string ms = "";
		std::string as = "";
		std::string ps = "";

		BlendStateID blendID[8] = {};
		RootSignatureConfig rootConfig = {};
		RasterizerID rasterizerID = RasterizerID::Fill;
		Topology topology = Topology::Triangle;
		DepthStencilID depthStencilID = DepthStencilID::Default;
		uint32_t rtvNum = 1;
		std::vector<DXGI_FORMAT> rtvFormats = { DXGI_FORMAT_R8G8B8A8_UNORM };
		DXGI_FORMAT dsvFormat = DXGI_FORMAT_D32_FLOAT;

		bool operator==(const ConfigMSType& other) const {
			return ms == other.ms &&
				ps == other.ps &&
				std::equal(std::begin(blendID), std::end(blendID), std::begin(other.blendID)) &&
				rootConfig == other.rootConfig &&
				rasterizerID == other.rasterizerID &&
				topology == other.topology &&
				depthStencilID == other.depthStencilID &&
				rtvNum == other.rtvNum;
		}
		bool operator!=(const ConfigMSType& other) const {
			return !(*this == other);
		}
		bool operator<(const ConfigMSType& other) const {
			if (ms != other.ms) return ms < other.ms;
			if (ps != other.ps) return ps < other.ps;
			for (size_t i = 0; i < 8; ++i) {
				if (blendID[i] != other.blendID[i]) return blendID[i] < other.blendID[i];
			}
			if (rootConfig != other.rootConfig) return rootConfig < other.rootConfig;
			if (rasterizerID != other.rasterizerID) return rasterizerID < other.rasterizerID;
			if (topology != other.topology) return topology < other.topology;
			if (depthStencilID != other.depthStencilID) return depthStencilID < other.depthStencilID;
			return rtvNum < other.rtvNum;
		}
	};

	// MeshShaderを使用するPSOを管理するクラス
	class ManagerMSType {
	public:

		ManagerMSType(DXDevice* device, ShelfManager* shelfManager);

		void Initialize();

		/// @brief PSOを設定する。初めて使用するPSOの場合は作成する
		void SetPSO(const ConfigMSType& config, ID3D12GraphicsCommandList* cmdList);

	private:

		struct PipelineStateStream {
			CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE rootSignature;
			CD3DX12_PIPELINE_STATE_STREAM_MS ms;
			CD3DX12_PIPELINE_STATE_STREAM_PS ps;
			CD3DX12_PIPELINE_STATE_STREAM_BLEND_DESC blend;
			CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL depth;
			CD3DX12_PIPELINE_STATE_STREAM_RASTERIZER rasterizer;
			CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS rtvFormats;
			CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT dsvFormat;
			CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_DESC sampleDesc;
			CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_MASK sampleMask;
			CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY primitiveType;
		};

		DXDevice* device_ = nullptr;  ///< DirectX12デバイス

		ShelfManager* shelfManager_ = nullptr;  ///< シェルフマネージャー
		std::map<ConfigMSType, Microsoft::WRL::ComPtr<ID3D12PipelineState>> psoMap_;  ///< PSOの設定とID3D12PipelineStateのマップ

	};

}
