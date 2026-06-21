#include "RootSignatureShelf.h"
#include <cassert>

using namespace SHEngine::PSO;

RootSignatureShelf::RootSignatureShelf(ID3D12Device* device) {
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
	uint32_t registerIndex;

	// === CBV ===
	//VertexShader
	registerIndex = 0;
	for (int i = 0; i < config.cbvNums.first; ++i) {
		auto& param = rootParameters.emplace_back();
		param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;        //CBVを使う
		param.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;    //VertexShaderで使う
		param.Descriptor.ShaderRegister = registerIndex++;          //レジスタ番号0とバインド
	}
	registerIndex = 0;
	for (int i = 0; i < config.cbvNums.second; ++i) {
		auto& param = rootParameters.emplace_back();
		param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;        //CBVを使う
		param.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;     //PixelShaderで使う
		param.Descriptor.ShaderRegister = registerIndex++;          //レジスタ番号0とバインド
	}

	// === SRV ===
	//VertexShader
	std::vector< D3D12_DESCRIPTOR_RANGE> vertexSrvRanges;
	vertexSrvRanges.resize(config.srvNums.first);
	int srvVSRegister = 0;			//いろんな奴と共用するので変数で明示化する
	for (int i = 0; i < config.srvNums.first; ++i) {
		auto& range = vertexSrvRanges[i];
		range.BaseShaderRegister = srvVSRegister++;
		range.NumDescriptors = 1;
		range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		auto& param = rootParameters.emplace_back();
		param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;       //SRVを使う
		param.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;                //VertexShaderで使う
		param.DescriptorTable.pDescriptorRanges = &range;						//テーブルの中身
		param.DescriptorTable.NumDescriptorRanges = 1;							//テーブルの数
	}

	//PixelShader
	std::vector< D3D12_DESCRIPTOR_RANGE> pixelSrvRanges;
	pixelSrvRanges.resize(config.srvNums.second);
	int srvPSRegister = 0;			//いろんな奴と共用するので変数で明示化する
	for (int i = 0; i < config.srvNums.second; ++i) {
		D3D12_DESCRIPTOR_RANGE& range = pixelSrvRanges[i];
		range.BaseShaderRegister = srvPSRegister++;
		range.NumDescriptors = 1;
		range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		auto& param = rootParameters.emplace_back();
		param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;       //SRVを使う
		param.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;                 //PixelShaderで使う
		param.DescriptorTable.pDescriptorRanges = &range;                       //テーブルの中身
		param.DescriptorTable.NumDescriptorRanges = 1;                          //テーブルの数
	}

	// === UAV ===
	//VertexShader
	std::vector< D3D12_DESCRIPTOR_RANGE> vertexUavRanges;
	vertexUavRanges.resize(config.uavNums.first);
	for (int i = 0; i < config.uavNums.first; ++i) {
		auto& range = vertexUavRanges[i];
		range.BaseShaderRegister = i;
		range.NumDescriptors = 1;
		range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
		range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		auto& param = rootParameters.emplace_back();
		param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;       //SRVを使う
		param.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;                //VertexShaderで使う
		param.DescriptorTable.pDescriptorRanges = &range;						//テーブルの中身
		param.DescriptorTable.NumDescriptorRanges = 1;							//テーブルの数
	}

	//PixelShader
	std::vector< D3D12_DESCRIPTOR_RANGE> pixelUavRanges;
	pixelUavRanges.resize(config.uavNums.second);
	for (int i = 0; i < config.uavNums.second; ++i) {
		D3D12_DESCRIPTOR_RANGE& range = pixelUavRanges[i];
		range.BaseShaderRegister = i;
		range.NumDescriptors = 1;
		range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
		range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		auto& param = rootParameters.emplace_back();
		param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;       //SRVを使う
		param.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;                 //PixelShaderで使う
		param.DescriptorTable.pDescriptorRanges = &range;                       //テーブルの中身
		param.DescriptorTable.NumDescriptorRanges = 1;                          //テーブルの数
	}

	// === 2DTexture ===
	//VertexShader
	std::vector< D3D12_DESCRIPTOR_RANGE> vertexTexture2DRanges;
	vertexTexture2DRanges.resize(config.textureNums.first);
	for (int i = 0; i < config.textureNums.first; ++i) {
		auto& range = vertexTexture2DRanges[i];
		range.BaseShaderRegister = srvVSRegister++;
		range.NumDescriptors = 1;
		range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		auto& param = rootParameters.emplace_back();
		param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;       //SRVを使う
		param.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;                //VertexShaderで使う
		param.DescriptorTable.pDescriptorRanges = &range;						//テーブルの中身
		param.DescriptorTable.NumDescriptorRanges = 1;							//テーブルの数
	}

	//PixelShader
	std::vector< D3D12_DESCRIPTOR_RANGE> pixelTexture2DRanges;
	pixelTexture2DRanges.resize(config.textureNums.second);
	for (int i = 0; i < config.textureNums.second; ++i) {
		D3D12_DESCRIPTOR_RANGE& range = pixelTexture2DRanges[i];
		range.BaseShaderRegister = srvPSRegister++;
		range.NumDescriptors = 1;
		range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		auto& param = rootParameters.emplace_back();
		param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;       //SRVを使う
		param.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;                 //PixelShaderで使う
		param.DescriptorTable.pDescriptorRanges = &range;                       //テーブルの中身
		param.DescriptorTable.NumDescriptorRanges = 1;                          //テーブルの数
	}

	// === DDS ===
	//VertexShader
	std::vector< D3D12_DESCRIPTOR_RANGE> vertexDDSRanges;
	vertexDDSRanges.resize(config.ddsNums.first);
	for (int i = 0; i < config.ddsNums.first; ++i) {
		auto& range = vertexDDSRanges[i];
		range.BaseShaderRegister = srvVSRegister++;
		range.NumDescriptors = 1;
		range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		auto& param = rootParameters.emplace_back();
		param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;       //SRVを使う
		param.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;                //VertexShaderで使う
		param.DescriptorTable.pDescriptorRanges = &range;						//テーブルの中身
		param.DescriptorTable.NumDescriptorRanges = 1;							//テーブルの数
	}

	//PixelShader
	std::vector< D3D12_DESCRIPTOR_RANGE> pixelDDSRanges;
	pixelDDSRanges.resize(config.ddsNums.second);
	for (int i = 0; i < config.ddsNums.second; ++i) {
		D3D12_DESCRIPTOR_RANGE& range = pixelDDSRanges[i];
		range.BaseShaderRegister = srvPSRegister++;
		range.NumDescriptors = 1;
		range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		auto& param = rootParameters.emplace_back();
		param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;       //SRVを使う
		param.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;                 //PixelShaderで使う
		param.DescriptorTable.pDescriptorRanges = &range;                       //テーブルの中身
		param.DescriptorTable.NumDescriptorRanges = 1;                          //テーブルの数
	}

	// === TextureList ===
	D3D12_DESCRIPTOR_RANGE textureDescriptor[1] = {};
	if (config.useTexture) {
		textureDescriptor[0].BaseShaderRegister = 8;
		textureDescriptor[0].NumDescriptors = 1024;
		textureDescriptor[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		textureDescriptor[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		auto& param = rootParameters.emplace_back();
		param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;           //SRVを使う
		param.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;                       //全Shaderから見れる
		param.DescriptorTable.pDescriptorRanges = textureDescriptor;                //テーブルの中身
		param.DescriptorTable.NumDescriptorRanges = _countof(textureDescriptor);    //テーブルの数
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

}

bool RootSignatureConfig::operator<(const RootSignatureConfig& other) const {
	if (cbvNums != other.cbvNums) return cbvNums < other.cbvNums;
	if (srvNums != other.srvNums) return srvNums < other.srvNums;
	if (useTexture != other.useTexture) return useTexture < other.useTexture;
	return samplers < other.samplers;
}

bool RootSignatureConfig::operator==(const RootSignatureConfig& other) const {
	return cbvNums == other.cbvNums &&
		srvNums == other.srvNums &&
		useTexture == other.useTexture &&
		samplers == other.samplers;
}
