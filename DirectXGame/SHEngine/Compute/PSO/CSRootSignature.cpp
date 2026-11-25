#include "CSRootSignature.h"
#include <set>

using namespace SHEngine::PSO;

void CSRootSignature::Initialize(DXDevice* device, std::map<SamplerID, D3D12_STATIC_SAMPLER_DESC> samplers) {
	device_ = device;
	samplers_ = samplers;
	rootSignatures_.clear();
}

ID3D12RootSignature* SHEngine::PSO::CSRootSignature::GetRootSignature(const RootSignatureConfig& config) {
	const auto& it = rootSignatures_.find(config);
	if (it != rootSignatures_.end()) {
		return it->second.Get();
	}
	return CreateRootSignature(config);
}

ID3D12RootSignature* SHEngine::PSO::CSRootSignature::CreateRootSignature(const RootSignatureConfig& config) {
	D3D12_ROOT_SIGNATURE_DESC rsDesc{};
	std::vector<D3D12_ROOT_PARAMETER> rootParameters;
	rootParameters.reserve(config.rootParams.size() + 1);

	// ポインタを保持するため、途中で再確保されないように先に確保しておく
	std::vector<D3D12_DESCRIPTOR_RANGE> ranges;
	ranges.reserve(config.rootParams.size() + 1);

	// 同じレジスタが二重に使われていないかのチェック用
	std::set<std::pair<BufferType, int>> usedRegisters;

	for (const auto& rootParam : config.rootParams) {
		if (rootParam.shader != ShaderType::COMPUTE_SHADER) {
			logger_->error("CSRootSignature: RootParam shader type must be COMPUTE_SHADER.");
			assert(false && "CSRootSignature: invalid shader type");
		}
		if (!usedRegisters.insert({ rootParam.bufferType, rootParam.registerNumber }).second) {
			logger_->error("CSRootSignature: Register {} for buffer type {} is already used.", rootParam.registerNumber, static_cast<int>(rootParam.bufferType));
			assert(false && "CSRootSignature: register number conflict");
		}
		if (rootParam.bufferType == BufferType::SRV && config.useTexture && rootParam.registerNumber >= 8) {
			logger_->error("CSRootSignature: SRV register {} conflicts with texture table (t8~).", rootParam.registerNumber);
			assert(false && "CSRootSignature: SRV register conflicts with texture table");
		}

		auto& param = rootParameters.emplace_back();
		param.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		switch (rootParam.bufferType) {
		case BufferType::CBV:
			param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
			param.Descriptor.ShaderRegister = rootParam.registerNumber;
			param.Descriptor.RegisterSpace = 0;
			break;
		case BufferType::SRV:
		case BufferType::UAV:
		{
			auto& range = ranges.emplace_back();
			range.BaseShaderRegister = rootParam.registerNumber;
			range.NumDescriptors = 1;
			range.RangeType = rootParam.bufferType == BufferType::SRV ? D3D12_DESCRIPTOR_RANGE_TYPE_SRV : D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
			range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

			param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			param.DescriptorTable.pDescriptorRanges = &range;
			param.DescriptorTable.NumDescriptorRanges = 1;
		}
		break;
		default:
			logger_->error("CSRootSignature: BufferType must be CBV, SRV or UAV.");
			assert(false && "CSRootSignature: invalid buffer type");
			break;
		}
	}

	if (config.useTexture) {
		auto& range = ranges.emplace_back();
		range.BaseShaderRegister = 8; // t8~t1031を使用する
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
	if (config.samplers > 0) {
		for (const auto& [id, desc] : samplers_) {
			if (config.samplers & uint32_t(id)) {
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
		if (errorBlob) {
			logger_->error(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		}
		assert(false && "Failed to serialize root signature");
		return nullptr;
	}

	//バイナリをもとに生成
	auto& rootSignature = rootSignatures_[config];
	hr = device_->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(),
		signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(hr) && "Failed to create root signature");

	return rootSignature.Get();
}
