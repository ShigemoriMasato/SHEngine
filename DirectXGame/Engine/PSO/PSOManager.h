#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <dxcapi.h>
#include <vector>
#include <Tools/Binary/BinaryManager.h>
#include <PSO/PSOConfig.h>

//あらゆるPSOを管理するクラス
class PSOManager {
public:

	PSOManager(ID3D12Device* device);
	~PSOManager();

	void Initialize();
	
	ID3D12PipelineState* GetPSO(const PSOConfig& config);

	ID3D12RootSignature* GetRootSignature(const RootSignatureID id) const;

	ShaderShelf* GetShaderShelf() const { return shaderShelf_.get(); }

private:

	void CreatePSO(PSOConfig config);

	ID3D12Device* device_ = nullptr;

	std::unordered_map<PSOConfig, ID3D12PipelineState*> psoMap_;

	std::unique_ptr<ShaderShelf> shaderShelf_{};
	std::unique_ptr<BlendStateShelf> blendStateShelf_{};
	std::unique_ptr<DepthStencilShelf> depthStencilShelf_{};
	std::unique_ptr<RasterizerShelf> rasterizerShelf_{};
	std::unique_ptr<RootSignatureShelf> rootSignatureShelf_{};
	std::unique_ptr<InputLayoutShelf> inputLayoutShelf_{};

	std::unique_ptr<BinaryManager> binaryManager_ = nullptr;
	const std::string shaderDataFile = "ShaderEditData.bin";

	std::shared_ptr<spdlog::logger> logger_ = nullptr;

	static std::unordered_map<D3D12_PRIMITIVE_TOPOLOGY, D3D12_PRIMITIVE_TOPOLOGY_TYPE> topologyMap_;
};
