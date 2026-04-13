#include "ShaderShelf.h"
#include <Utility/ConvertString.h>
#include <Utility/SearchFile.h>
#include <Core/DXDevice.h>
#include <cassert>

using namespace SHEngine::PSO;

namespace fs = std::filesystem;

ShaderShelf::ShaderShelf(DXDevice* device) {
	device_ = device;

	compileVersions_[0] = L"vs_6_0"; // Vertex Shader
	compileVersions_[1] = L"ps_6_0"; // Pixel Shader
}

ShaderShelf::~ShaderShelf() {

}

void ShaderShelf::CompileAllShader() {

	std::vector<std::string> shaderNames;

	//shaderNameBufferを削除するためのスコープ
	{
		std::vector<std::string> shaderNameBuffer = SearchFiles(basePath_, ".hlsl");
		for (const auto& name : shaderNameBuffer) {
			shaderNames.push_back(name);
		}
	}

	for (const auto& sn : shaderNames) {
		if (sn.find(".hlsli") != std::string::npos) {
			continue;
		}
		//VertexShaderだったら
		else if (sn.find("VS") != std::string::npos) {
			RegisterShaderByteCode(sn, ShaderType::VERTEX_SHADER);
		}
		//PixelShaderだったら
		else if (sn.find("PS") != std::string::npos) {
			RegisterShaderByteCode(sn, ShaderType::PIXEL_SHADER);
		}
		//これ以降も同じように追加する


		// 登録してない、または名前が間違っているShaderは見つかり次第エラーを出す
		else {
			throw std::runtime_error("Unknown shader type: " + sn);
		}
	}
}

std::list<D3D12_SHADER_BYTECODE> ShaderShelf::GetShaderBytecodes(ShaderType shaderType) {
	std::list<D3D12_SHADER_BYTECODE> ans;

	for (const auto& [name, bytecode] : shaderBytecodes_[static_cast<size_t>(shaderType)]) {
		ans.push_back(bytecode);
	}

	return ans;
}

std::list<std::string> ShaderShelf::GetShaderNames(ShaderType shaderType) {
	std::list<std::string> ans;

	for (const auto& [name, bytecode] : shaderBytecodes_[static_cast<size_t>(shaderType)]) {
		ans.push_back(name);
	}

	return ans;
}

D3D12_SHADER_BYTECODE ShaderShelf::GetShaderBytecode(ShaderType shaderType, std::string shaderName) {
	//登録されていなければCompileする
	if (shaderBytecodes_[static_cast<int>(shaderType)][shaderName].pShaderBytecode == D3D12_SHADER_BYTECODE().pShaderBytecode) {
		RegisterShaderByteCode(shaderName, shaderType);
	}
	return shaderBytecodes_[static_cast<int>(shaderType)][shaderName];
}

void ShaderShelf::RegisterShaderByteCode(std::string shaderName, ShaderType shaderType) {
	IDxcBlob* blobBuffer = device_->CompileShader(basePath_.string() + "/" + shaderName, shaderType);

	shaderBlobs_.push_back(blobBuffer);

	shaderBytecodes_[int(shaderType)][shaderName] = { blobBuffer->GetBufferPointer(), blobBuffer->GetBufferSize() };
}
