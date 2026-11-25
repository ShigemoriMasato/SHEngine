#pragma once
#include <Core/DXDevice.h>
#include <Render/PSO/Shelf/RootSignatureShelf.h>

namespace SHEngine::PSO {

	class CSRootSignature {
	public:

		void Initialize(DXDevice* device, std::map<SamplerID, D3D12_STATIC_SAMPLER_DESC> samplers);

		// @brief 設定を基にComputeShader用のルートシグネチャを取得する。未作成なら作成してキャッシュする。
		// @param config rootParamsの順番がそのままRootParameterIndexになる。shaderはCOMPUTE_SHADERであること。
		ID3D12RootSignature* GetRootSignature(const RootSignatureConfig& config);

	private:

		ID3D12RootSignature* CreateRootSignature(const RootSignatureConfig& config);

		DXDevice* device_ = nullptr;

		std::map<RootSignatureConfig, Microsoft::WRL::ComPtr<ID3D12RootSignature>> rootSignatures_;
		std::map<SamplerID, D3D12_STATIC_SAMPLER_DESC> samplers_;

		Logger logger_ = GetLogger("Engine");
	};

}
