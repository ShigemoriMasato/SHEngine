#include "ShaderReflection.h"
#include <algorithm>
#include <format>

using namespace SHEngine;

namespace {

	// 上限なしの配列 (Texture2D tex[]) は BindCount が 0 か UINT_MAX で報告されるので 0 にそろえる
	uint32_t NormalizeCount(UINT bindCount) {
		if (bindCount == 0 || bindCount == UINT_MAX) {
			return 0;
		}
		return bindCount;
	}

	// StructuredBuffer系の1要素のサイズを取得する。
	// リソースと同名の D3D_CT_RESOURCE_BIND_INFO 型のバッファの0番の変数サイズがストライドになる。
	uint32_t GetStructureStride(ID3D12ShaderReflection* reflection, const char* name) {
		ID3D12ShaderReflectionConstantBuffer* cb = reflection->GetConstantBufferByName(name);
		if (!cb) {
			return 0;
		}
		D3D12_SHADER_BUFFER_DESC cbDesc{};
		if (FAILED(cb->GetDesc(&cbDesc)) || cbDesc.Type != D3D_CT_RESOURCE_BIND_INFO || cbDesc.Variables == 0) {
			return 0;
		}
		ID3D12ShaderReflectionVariable* var = cb->GetVariableByIndex(0);
		D3D12_SHADER_VARIABLE_DESC varDesc{};
		if (!var || FAILED(var->GetDesc(&varDesc))) {
			return 0;
		}
		return varDesc.Size;
	}

	// cbufferのサイズを取得する
	uint32_t GetConstantBufferSize(ID3D12ShaderReflection* reflection, const char* name) {
		ID3D12ShaderReflectionConstantBuffer* cb = reflection->GetConstantBufferByName(name);
		if (!cb) {
			return 0;
		}
		D3D12_SHADER_BUFFER_DESC cbDesc{};
		if (FAILED(cb->GetDesc(&cbDesc))) {
			return 0;
		}
		return cbDesc.Size;
	}

	template<class T>
	void SortByRegister(std::vector<T>& bindings) {
		std::sort(bindings.begin(), bindings.end(), [](const T& a, const T& b) {
			if (a.space != b.space) return a.space < b.space;
			return a.registerIndex < b.registerIndex;
			});
	}

	ShaderBufferBinding MakeBuffer(const D3D12_SHADER_INPUT_BIND_DESC& desc) {
		ShaderBufferBinding binding;
		binding.name = desc.Name ? desc.Name : "";
		binding.registerIndex = desc.BindPoint;
		binding.space = desc.Space;
		return binding;
	}

	ShaderTextureBinding MakeTexture(const D3D12_SHADER_INPUT_BIND_DESC& desc, bool isUAV) {
		ShaderTextureBinding binding;
		binding.name = desc.Name ? desc.Name : "";
		binding.registerIndex = desc.BindPoint;
		binding.space = desc.Space;
		binding.count = NormalizeCount(desc.BindCount);
		binding.dimension = desc.Dimension;
		binding.isUAV = isUAV;
		return binding;
	}

}

bool SHEngine::ShaderReflector::Build(ID3D12ShaderReflection* reflection, ShaderType shaderType, ShaderReflectionData& out) {
	out = {};
	if (!reflection) {
		return false;
	}

	D3D12_SHADER_DESC shaderDesc{};
	if (FAILED(reflection->GetDesc(&shaderDesc))) {
		return false;
	}

	out.shaderType = shaderType;

	if (shaderType == ShaderType::COMPUTE_SHADER || shaderType == ShaderType::MESH_SHADER || shaderType == ShaderType::AMPLIFICATION_SHADER) {
		UINT x = 0, y = 0, z = 0;
		reflection->GetThreadGroupSize(&x, &y, &z);
		out.threadGroupSize[0] = x;
		out.threadGroupSize[1] = y;
		out.threadGroupSize[2] = z;
	}

	for (UINT i = 0; i < shaderDesc.BoundResources; ++i) {
		D3D12_SHADER_INPUT_BIND_DESC bindDesc{};
		if (FAILED(reflection->GetResourceBindingDesc(i, &bindDesc))) {
			continue;
		}

		switch (bindDesc.Type) {
		case D3D_SIT_CBUFFER:
		{
			ShaderBufferBinding binding = MakeBuffer(bindDesc);
			binding.size = GetConstantBufferSize(reflection, bindDesc.Name);
			out.cbvs.push_back(binding);
		}
		break;

		// ---- SRV ----
		case D3D_SIT_TBUFFER:
		case D3D_SIT_BYTEADDRESS:
			out.srvs.push_back(MakeBuffer(bindDesc));
			break;
		case D3D_SIT_STRUCTURED:
		{
			ShaderBufferBinding binding = MakeBuffer(bindDesc);
			binding.size = GetStructureStride(reflection, bindDesc.Name);
			out.srvs.push_back(binding);
		}
		break;
		case D3D_SIT_TEXTURE:
			// Buffer<T> はテクスチャではなくバッファとして扱う
			if (bindDesc.Dimension == D3D_SRV_DIMENSION_BUFFER) {
				out.srvs.push_back(MakeBuffer(bindDesc));
			} else {
				out.textures.push_back(MakeTexture(bindDesc, false));
			}
			break;

		// ---- UAV ----
		case D3D_SIT_UAV_RWBYTEADDRESS:
			out.uavs.push_back(MakeBuffer(bindDesc));
			break;
		case D3D_SIT_UAV_RWSTRUCTURED:
		case D3D_SIT_UAV_APPEND_STRUCTURED:
		case D3D_SIT_UAV_CONSUME_STRUCTURED:
		case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
		{
			ShaderBufferBinding binding = MakeBuffer(bindDesc);
			binding.size = GetStructureStride(reflection, bindDesc.Name);
			binding.hasCounter = bindDesc.Type != D3D_SIT_UAV_RWSTRUCTURED;
			out.uavs.push_back(binding);
		}
		break;
		case D3D_SIT_UAV_RWTYPED:
			// RWBuffer<T> はバッファ、RWTexture は テクスチャ
			if (bindDesc.Dimension == D3D_SRV_DIMENSION_BUFFER) {
				out.uavs.push_back(MakeBuffer(bindDesc));
			} else {
				out.textures.push_back(MakeTexture(bindDesc, true));
			}
			break;

		// ---- Sampler ----
		case D3D_SIT_SAMPLER:
		{
			ShaderSamplerBinding binding;
			binding.name = bindDesc.Name ? bindDesc.Name : "";
			binding.registerIndex = bindDesc.BindPoint;
			binding.space = bindDesc.Space;
			binding.count = NormalizeCount(bindDesc.BindCount);
			out.samplers.push_back(binding);
		}
		break;

		default:
			// RaytracingAccelerationStructure などは未対応
			break;
		}
	}

	SortByRegister(out.cbvs);
	SortByRegister(out.srvs);
	SortByRegister(out.uavs);
	SortByRegister(out.textures);
	SortByRegister(out.samplers);

	return true;
}

std::string SHEngine::ShaderReflectionData::ToString() const {
	std::string str;
	auto bufferLine = [&](char prefix, const ShaderBufferBinding& b) {
		str += std::format("  {}{} (space{}) {} size:{}{}\n", prefix, b.registerIndex, b.space, b.name, b.size, b.hasCounter ? " counter" : "");
		};

	str += std::format("numthreads({}, {}, {})\n", threadGroupSize[0], threadGroupSize[1], threadGroupSize[2]);
	str += "CBV:\n";
	for (const auto& b : cbvs) bufferLine('b', b);
	str += "SRV:\n";
	for (const auto& b : srvs) bufferLine('t', b);
	str += "UAV:\n";
	for (const auto& b : uavs) bufferLine('u', b);
	str += "Texture:\n";
	for (const auto& t : textures) {
		str += std::format("  {}{} (space{}) {} count:{} dim:{}\n", t.isUAV ? 'u' : 't', t.registerIndex, t.space, t.name, t.count, int(t.dimension));
	}
	str += "Sampler:\n";
	for (const auto& s : samplers) {
		str += std::format("  s{} (space{}) {} count:{}\n", s.registerIndex, s.space, s.name, s.count);
	}
	return str;
}
