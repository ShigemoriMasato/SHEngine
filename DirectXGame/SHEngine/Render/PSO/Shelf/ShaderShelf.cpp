#include "ShaderShelf.h"
#include <Utility/ConvertString.h>
#include <Utility/SearchFile.h>
#include <Core/DXDevice.h>
#include <cassert>

using namespace SHEngine::PSO;

namespace fs = std::filesystem;

ShaderShelf::ShaderShelf(DXDevice* device) {
	device_ = device;

	compileVersions_[int(ShaderType::VERTEX_SHADER)] = L"vs_6_5"; // Vertex Shader
	compileVersions_[int(ShaderType::PIXEL_SHADER)] = L"ps_6_5"; // Pixel Shader
	compileVersions_[int(ShaderType::COMPUTE_SHADER)] = L"cs_6_5"; // Pixel Shader
	compileVersions_[int(ShaderType::MESH_SHADER)] = L"ms_6_5"; // Pixel Shader
}

ShaderShelf::~ShaderShelf() {
	ReleaseShader("All");
}

void ShaderShelf::CompileAllShader() {

	std::vector<std::string> shaderNames;

	//shaderNameBufferを削除するためのスコープ
	{
		std::vector<std::string> shaderNameBuffer = SearchFilePathsAddChild(basePath_, ".hlsl");
		for (const auto& name : shaderNameBuffer) {
			shaderNames.push_back(name);
		}
	}

	ShaderType shaderType = ShaderType::VERTEX_SHADER;

	for (const auto& sn : shaderNames) {
		if (sn.find(".hlsli") != std::string::npos) {
			continue;
		}

		shaderType = GetShaderTypeFromFileName(sn);

		// 対応するshaderであればコンパイルして登録する
		if (shaderType != ShaderType::Count) {
			RegisterShaderByteCode(sn, shaderType);
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
	const auto& it = shaderBytecodes_[static_cast<int>(shaderType)].find(shaderName);
	if (it == shaderBytecodes_[static_cast<int>(shaderType)].end()) {
		RegisterShaderByteCode(shaderName, shaderType);
	}
	return shaderBytecodes_[static_cast<int>(shaderType)][shaderName];
}

void SHEngine::PSO::ShaderShelf::ShaderReset(const std::string& shaderName) {
	ReleaseShader(shaderName);
}

void ShaderShelf::RegisterShaderByteCode(std::string shaderName, ShaderType shaderType) {
	IDxcBlob* blobBuffer = device_->CompileShader(basePath_.string() + "/" + shaderName, shaderType);

	// コンパイルに失敗した場合は空のバイトコードを登録する
	if (!blobBuffer) {
		shaderBytecodes_[int(shaderType)][shaderName] = D3D12_SHADER_BYTECODE();
		return;
	}

	shaderBlobs_.push_back(blobBuffer);

	shaderBytecodes_[int(shaderType)][shaderName] = { blobBuffer->GetBufferPointer(), blobBuffer->GetBufferSize() };
}

void SHEngine::PSO::ShaderShelf::ReleaseShader(std::string shaderName) {
	if (shaderName == "All") {
		//全て再コンパイルは探索の必要がなく、さらにメモリの圧迫の危険になるため、Blobまで開放する
		for (auto& blob : shaderBlobs_) {
			if (blob) {
				blob->Release();
				blob = nullptr;
			}
		}
		for (int i = 0; i < static_cast<int>(ShaderType::Count); ++i) {
			shaderBytecodes_[i].clear();
		}
	} else {
		// 探索が困難なため、Blobの開放は行わない
		ShaderType shaderType = GetShaderTypeFromFileName(shaderName);

		if (shaderType != ShaderType::Count) {
			shaderBytecodes_[int(shaderType)].erase(shaderName);
		}
	}
}

ShaderType SHEngine::PSO::ShaderShelf::GetShaderTypeFromFileName(const std::string& fileName) {
	ShaderType shaderType;

	if (fileName.find("VS") != std::string::npos) {
		shaderType = ShaderType::VERTEX_SHADER;
	}
	//PixelShader
	else if (fileName.find("PS") != std::string::npos) {
		shaderType = ShaderType::PIXEL_SHADER;
	}
	//ComputeShader
	else if (fileName.find("CS") != std::string::npos) {
		shaderType = ShaderType::COMPUTE_SHADER;
	}
	//MeshShader
	else if (fileName.find("MS") != std::string::npos) {
		shaderType = ShaderType::MESH_SHADER;
	}
	//AmplificationShader
	else if (fileName.find("AS") != std::string::npos) {
		shaderType = ShaderType::AMPLIFICATION_SHADER;
	}
	// 登録してない、または名前が間違っているShaderは見つかり次第エラーを出す
	else {
		shaderType = ShaderType::Count;
	}

	return shaderType;
}
