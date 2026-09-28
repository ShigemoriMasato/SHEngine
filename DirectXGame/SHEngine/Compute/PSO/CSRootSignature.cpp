#include "CSRootSignature.h"

using namespace SHEngine::PSO;

void CSRootSignature::Initialize(DXDevice* device, std::map<SamplerID, D3D12_STATIC_SAMPLER_DESC> samplers) {
	device_ = device;
	samplers_ = samplers;
	rootSignatures_.clear();
}

ID3D12RootSignature* SHEngine::PSO::CSRootSignature::GetRootSignature(int cbv, int srv, int uav, bool useTexture, uint32_t samplerID) {
	RSConfig config{ cbv, srv, uav };
	const auto& it = rootSignatures_.find(config);
	if (it != rootSignatures_.end()) {
		return it->second.rootSignature.Get();
	}

	D3D12_ROOT_SIGNATURE_DESC rsDesc{};
	std::vector<D3D12_ROOT_PARAMETER> rootParameters;
	rootParameters.reserve(cbv + srv + uav);
	for(int i = 0; i < cbv; ++i) {
		auto& param = rootParameters.emplace_back();
		param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		param.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		param.Descriptor.ShaderRegister = i;
	}

	std::vector<D3D12_DESCRIPTOR_RANGE> srvRanges;
	srvRanges.reserve(srv);
	for (int i = 0; i < srv; ++i) {
		auto& range = srvRanges.emplace_back();
		range.BaseShaderRegister = i;
		range.NumDescriptors = 1;
		range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		auto& param = rootParameters.emplace_back();
		param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		param.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		param.DescriptorTable.pDescriptorRanges = &range;
		param.DescriptorTable.NumDescriptorRanges = 1;
	}

	std::vector<D3D12_DESCRIPTOR_RANGE> uavRanges;
	uavRanges.reserve(uav);
	for(int i = 0; i < uav; ++i) {
		auto& range = uavRanges.emplace_back();
		range.BaseShaderRegister = i;
		range.NumDescriptors = 1;
		range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
		range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		auto& param = rootParameters.emplace_back();
		param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		param.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		param.DescriptorTable.pDescriptorRanges = &range;
		param.DescriptorTable.NumDescriptorRanges = 1;
	}

	D3D12_DESCRIPTOR_RANGE textureRange{};
	if (useTexture) {
		auto& range = textureRange;
		range.BaseShaderRegister = 8; // t8~t1032を使用する
		range.NumDescriptors = 1024;
		range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		auto& param = rootParameters.emplace_back();
		param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		param.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		param.DescriptorTable.pDescriptorRanges = &range;
		param.DescriptorTable.NumDescriptorRanges = 1;
	}

	std::vector<D3D12_STATIC_SAMPLER_DESC> staticSamplers;
	if (samplerID > 0) {

		for (const auto& [id, desc] : samplers_) {
			if (samplerID & uint32_t(id)) {
				staticSamplers.push_back(desc);
			}
		}

		rsDesc.NumStaticSamplers = UINT(staticSamplers.size());
		rsDesc.pStaticSamplers = staticSamplers.data();
	}

	Microsoft::WRL::ComPtr<ID3D10Blob> signatureBlob = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
	//まとめる
	rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;
	rsDesc.NumParameters = UINT(rootParameters.size());
	rsDesc.pParameters = rootParameters.data();

	//シリアライズしてバイナリにする
	HRESULT hr = D3D12SerializeRootSignature(&rsDesc,
		D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);

	if (FAILED(hr)) {
		assert(false && "Failed to serialize root signature");
	}

	//バイナリをもとに生成
	auto& rootSignature = rootSignatures_[config].rootSignature;
	hr = device_->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(),
		signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(hr) && "Failed to create root signature");

	return rootSignature.Get();
}
