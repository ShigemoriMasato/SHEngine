#include "RootSignatureShelf.h"
#include <cassert>
#include <unordered_set>

using namespace SHEngine::PSO;

RootSignatureShelf::RootSignatureShelf(ID3D12Device2* device) {
	logger_ = GetLogger("Engine");
	device_ = device;
	rootSignatureMap_.clear();

#pragma region CreateSamplers

	D3D12_STATIC_SAMPLER_DESC base{};
	base.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	base.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	base.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	base.MinLOD = 0.0f;
	base.MaxLOD = D3D12_FLOAT32_MAX;
	base.MipLODBias = 0.0f;
	base.MaxAnisotropy = 1;
	base.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	base.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	base.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	base.ShaderRegister = 0;
	base.RegisterSpace = 0;
	base.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	D3D12_STATIC_SAMPLER_DESC def = base;
	samplers_[SamplerID::Default] = def;

	D3D12_STATIC_SAMPLER_DESC clampT = base;
	clampT.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplers_[SamplerID::ClampT] = clampT;

	D3D12_STATIC_SAMPLER_DESC mirrorT = base;
	mirrorT.AddressV = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
	samplers_[SamplerID::MirrorT] = mirrorT;

	D3D12_STATIC_SAMPLER_DESC clampS = base;
	clampS.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplers_[SamplerID::ClampS] = clampS;

	D3D12_STATIC_SAMPLER_DESC mirrorS = base;
	mirrorS.AddressU = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
	samplers_[SamplerID::MirrorS] = mirrorS;

	D3D12_STATIC_SAMPLER_DESC magNearest = base;
	magNearest.Filter = D3D12_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
	samplers_[SamplerID::MagNearest] = magNearest;

	D3D12_STATIC_SAMPLER_DESC magLinear = base;
	magLinear.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	samplers_[SamplerID::MagLinear] = magLinear;

	D3D12_STATIC_SAMPLER_DESC minNearest = base;
	minNearest.Filter = D3D12_FILTER_MIN_POINT_MAG_MIP_LINEAR;
	samplers_[SamplerID::MinNearest] = minNearest;

	D3D12_STATIC_SAMPLER_DESC minLinear = base;
	minLinear.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	samplers_[SamplerID::MinLinear] = minLinear;

	D3D12_STATIC_SAMPLER_DESC minNearestMipNearest = base;
	minNearestMipNearest.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	samplers_[SamplerID::MinNearestMipmapNearest] = minNearestMipNearest;

	D3D12_STATIC_SAMPLER_DESC minLinearMipNearest = base;
	minLinearMipNearest.Filter = D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR;
	samplers_[SamplerID::MinLinearMipmapNearest] = minLinearMipNearest;

	D3D12_STATIC_SAMPLER_DESC minNearestMipLinear = base;
	minNearestMipLinear.Filter = D3D12_FILTER_MIN_POINT_MAG_MIP_LINEAR;
	samplers_[SamplerID::MinNearestMipmapLinear] = minNearestMipLinear;

	D3D12_STATIC_SAMPLER_DESC minLinearMipLinear = base;
	minLinearMipLinear.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	samplers_[SamplerID::MinLinearMipmapLinear] = minLinearMipLinear;

	D3D12_STATIC_SAMPLER_DESC clampClampMinMagNearest = base;
	clampClampMinMagNearest.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	clampClampMinMagNearest.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	clampClampMinMagNearest.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	samplers_[SamplerID::ClampClamp_MinMagNearest] = clampClampMinMagNearest;

	D3D12_STATIC_SAMPLER_DESC point = base;
	point.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	samplers_[SamplerID::Point] = point;

#pragma endregion
}

RootSignatureShelf::~RootSignatureShelf() {
	for (auto& [config, rootSignature] : rootSignatureMap_) {
		if (rootSignature) {
			rootSignature->Release();
		}
	}
}

ID3D12RootSignature* RootSignatureShelf::GetRootSignature(const RootSignatureConfig& config) {
	auto it = rootSignatureMap_.find(config);
	if (it == rootSignatureMap_.end()) {
		return CreateRootSignature(config);
	}

	return rootSignatureMap_[config];
}



ID3D12RootSignature* RootSignatureShelf::CreateRootSignature(const RootSignatureConfig& config) {
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	//RootParameter作成
	std::vector<D3D12_ROOT_PARAMETER> rootParameters = {};

	// 使用済みのレジスタ番号を追跡するセット
	std::unordered_map<ShaderType, std::unordered_map<BufferType, std::unordered_set<int>>> usedRegisters; 
	static std::unordered_map<ShaderType, D3D12_SHADER_VISIBILITY> shaderVisibilityMap = {
		{ShaderType::VERTEX_SHADER, D3D12_SHADER_VISIBILITY_VERTEX},
		{ShaderType::PIXEL_SHADER, D3D12_SHADER_VISIBILITY_PIXEL},
		{ShaderType::COMPUTE_SHADER, D3D12_SHADER_VISIBILITY_ALL},
		{ShaderType::MESH_SHADER, D3D12_SHADER_VISIBILITY_MESH}
	};

	auto registerChecker = [&](const RootParam& config) {
		if (usedRegisters[config.shader][config.bufferType].count(config.registerNumber) > 0) {
			logger_->error("Register {} for shader {} and buffer type {} is already used.", config.registerNumber, static_cast<int>(config.shader), static_cast<int>(config.bufferType));
			assert(false && "Register number conflict detected.");
		}
		if (config.bufferType == BufferType::SRV && config.registerNumber >= 8) {
			logger_->error("SRV register number {} exceeds the limit of 7.", config.registerNumber);
			assert(false && "SRV register number exceeds the limit.");
		}
		usedRegisters[config.shader][config.bufferType].insert(config.registerNumber);
		};

	std::vector<D3D12_DESCRIPTOR_RANGE> descriptorRanges;
	descriptorRanges.reserve(config.rootParams.size());
	for (const auto& config : config.rootParams) {

		registerChecker(config);

		auto& param = rootParameters.emplace_back();

		switch (config.bufferType) {
		case BufferType::CBV:
		{
			param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
			param.ShaderVisibility = shaderVisibilityMap[config.shader];
			param.Descriptor.ShaderRegister = config.registerNumber;
		}
		break;
		case BufferType::SRV:
		{
			auto& range = descriptorRanges.emplace_back();
			range.BaseShaderRegister = config.registerNumber;
			range.NumDescriptors = 1;
			range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
			range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

			param.DescriptorTable.NumDescriptorRanges = 1;
			param.DescriptorTable.pDescriptorRanges = &range;
			param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			param.ShaderVisibility = shaderVisibilityMap[config.shader];
		}
		break;
		case BufferType::UAV:
		{
			auto& range = descriptorRanges.emplace_back();
			range.BaseShaderRegister = config.registerNumber;
			range.NumDescriptors = 1;
			range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
			range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

			param.DescriptorTable.NumDescriptorRanges = 1;
			param.DescriptorTable.pDescriptorRanges = &range;
			param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			param.ShaderVisibility = shaderVisibilityMap[config.shader];
		}
		}
	}

	// === TextureList ===
	if (config.useTexture) {
		auto& textureDescriptor = descriptorRanges.emplace_back();

		textureDescriptor.BaseShaderRegister = 8;
		textureDescriptor.NumDescriptors = 1024;
		textureDescriptor.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		textureDescriptor.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		auto& param = rootParameters.emplace_back();
		param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;           //SRVを使う
		param.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;                       //全Shaderから見れる
		param.DescriptorTable.pDescriptorRanges = &textureDescriptor;               //テーブルの中身
		param.DescriptorTable.NumDescriptorRanges = 1;								//テーブルの数
	}

	descriptionRootSignature.NumParameters = UINT(rootParameters.size());
	descriptionRootSignature.pParameters = rootParameters.data();

	// === Sampler ===
	std::vector<D3D12_STATIC_SAMPLER_DESC> staticSamplers;
	if (config.samplers > 0) {

		for (const auto& [id, desc] : samplers_) {
			if (config.samplers & uint32_t(id)) {
				staticSamplers.push_back(desc);
			}
		}

		descriptionRootSignature.NumStaticSamplers = UINT(staticSamplers.size());
		descriptionRootSignature.pStaticSamplers = staticSamplers.data();
	}

	//作成
	Microsoft::WRL::ComPtr<ID3D10Blob> signatureBlob = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;

	//シリアライズしてバイナリにする
	HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignature,
		D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr)) {
		logger_->error(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}

	//バイナリをもとに生成
	ID3D12RootSignature* rootSignature = nullptr;
	hr = device_->CreateRootSignature(0, signatureBlob->GetBufferPointer(),
		signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(hr));

	rootSignatureMap_[config] = rootSignature;

	return rootSignature;
}

namespace SHEngine::PSO {

	uint32_t operator|(SamplerID a, SamplerID b) {
		return uint32_t(a) | uint32_t(b);
	}

	uint32_t operator|(uint32_t a, SamplerID b) {
		return a | uint32_t(b);
	}

	bool operator<(SamplerID a, SamplerID b) {
		return uint32_t(a) < uint32_t(b);
	}

	bool RootSignatureConfig::operator<(const RootSignatureConfig& other) const {
		bool isLess = false;
		isLess |= useTexture < other.useTexture;
		isLess |= samplers < other.samplers;
		isLess |= rootParams.size() < other.rootParams.size();

		if (!isLess) {
			//rootParams以外の要素が一致しており、これ以上の調査が必要かどうかを確かめる。
			bool sizeEqual = rootParams.size() == other.rootParams.size() && samplers == other.samplers && useTexture == other.useTexture;
			//sizeEqual == false : すでにもう片方がでかいことが確定しているので、ループを続ける必要はない。
			for (int i = 0; !(sizeEqual && i >= (int)rootParams.size()); ++i) {
				isLess |= rootParams[i].bufferType < other.rootParams[i].bufferType;
				isLess |= rootParams[i].registerNumber < other.rootParams[i].registerNumber;
				isLess |= rootParams[i].shader < other.rootParams[i].shader;
			}
		}
		return isLess;
	}

	bool RootSignatureConfig::operator==(const RootSignatureConfig& other) const {
		bool ans = useTexture == other.useTexture || samplers == other.samplers || rootParams.size() == other.rootParams.size();
		if (ans) {
			for (int i = 0; i < (int)rootParams.size(); ++i) {
				if (rootParams[i].bufferType != other.rootParams[i].bufferType ||
					rootParams[i].registerNumber != other.rootParams[i].registerNumber ||
					rootParams[i].shader != other.rootParams[i].shader) {

					ans = false;
					break;

				}
			}
		}
		return ans;
	}

}
