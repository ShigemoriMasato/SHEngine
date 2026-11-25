#include "ShaderShelf.h"
#include "ShaderCache.h"
#include <Utility/ConvertString.h>
#include <Utility/SearchFile.h>
#include <Core/DXDevice.h>
#include <cassert>

using namespace SHEngine::PSO;

namespace fs = std::filesystem;

ShaderShelf::ShaderShelf(DXDevice* device) {
	device_ = device;
}

ShaderShelf::~ShaderShelf() {
	ReleaseShader("All");
}

void ShaderShelf::CompileAllShader() {

	std::vector<std::string> shaderNames = SearchFilePathsAddChild(basePath_, ".hlsl");

	for (const auto& sn : shaderNames) {
		ShaderType shaderType = GetShaderTypeFromFileName(sn);

		// 対応するshaderであればコンパイルして登録する
		if (shaderType != ShaderType::Count) {
			RegisterShaderByteCode(sn, shaderType);
		} else {
			logger_->warn("ShaderShelf: Unknown shader type. File name must be \"Name.(VS|PS|CS|MS|AS).hlsl\" : {}", sn);
		}
	}
}

std::list<D3D12_SHADER_BYTECODE> ShaderShelf::GetShaderBytecodes(ShaderType shaderType) {
	std::list<D3D12_SHADER_BYTECODE> ans;

	for (const auto& [name, entry] : shaders_[static_cast<size_t>(shaderType)]) {
		ans.push_back(entry.bytecode);
	}

	return ans;
}

std::list<std::string> ShaderShelf::GetShaderNames(ShaderType shaderType) {
	std::list<std::string> ans;

	for (const auto& [name, entry] : shaders_[static_cast<size_t>(shaderType)]) {
		ans.push_back(name);
	}

	return ans;
}

ShaderShelf::ShaderEntry& ShaderShelf::GetOrRegister(ShaderType shaderType, const std::string& shaderName) {
	auto& shaders = shaders_[static_cast<int>(shaderType)];

	//登録されていなければCompileする
	auto it = shaders.find(shaderName);
	if (it == shaders.end()) {
		ShaderType fileType = GetShaderTypeFromFileName(shaderName);
		if (fileType != shaderType) {
			logger_->warn("ShaderShelf: Requested shader type does not match file extension. : {}", shaderName);
		}
		RegisterShaderByteCode(shaderName, shaderType);
		it = shaders.find(shaderName);
	}
	return it->second;
}

D3D12_SHADER_BYTECODE ShaderShelf::GetShaderBytecode(ShaderType shaderType, std::string shaderName) {
	return GetOrRegister(shaderType, shaderName).bytecode;
}

const SHEngine::ShaderReflectionData* ShaderShelf::GetShaderReflection(ShaderType shaderType, const std::string& shaderName) {
	const ShaderEntry& entry = GetOrRegister(shaderType, shaderName);
	if (entry.bytes.empty() || !entry.reflection.IsValid()) {
		return nullptr;
	}
	return &entry.reflection;
}

void SHEngine::PSO::ShaderShelf::ShaderReset(const std::string& shaderName) {
	ReleaseShader(shaderName);
}

void ShaderShelf::RegisterShaderByteCode(std::string shaderName, ShaderType shaderType) {
	auto& entry = shaders_[int(shaderType)][shaderName];
	entry = {};

	const std::string sourcePath = basePath_.string() + "/" + shaderName;

	// Compiled/Shader/ にキャッシュがあり、ソースと#includeしたファイルが更新されていなければそれを使う
	ShaderCache::CachedShader cached;
	if (ShaderCache::Load(shaderName, sourcePath, shaderType, cached)) {
		entry.bytes = std::move(cached.bytecode);
		entry.reflection = std::move(cached.reflection);
		logger_->info("Shader loaded from cache, path: {}", sourcePath);
	} else {
		// CompileShaderは参照カウント1のBlobを返すので、Attachで所有権を受け取る
		std::vector<std::string> includes;
		Microsoft::WRL::ComPtr<IDxcBlob> blob;
		blob.Attach(device_->CompileShader(sourcePath, shaderType, &entry.reflection, &includes));

		// コンパイルに失敗した場合は空のバイトコードを登録する
		if (!blob) {
			return;
		}

		const uint8_t* begin = static_cast<const uint8_t*>(blob->GetBufferPointer());
		entry.bytes.assign(begin, begin + blob->GetBufferSize());

		// 次回起動時用に保存する
		ShaderCache::Save(shaderName, sourcePath, shaderType, entry.bytes, entry.reflection, includes);
	}

	entry.bytecode = { entry.bytes.data(), entry.bytes.size() };
}

void SHEngine::PSO::ShaderShelf::ReleaseShader(std::string shaderName) {
	// PSOは作成時にバイトコードをコピーするため、作成済みのPSOに影響はない
	if (shaderName == "All") {
		for (auto& shaders : shaders_) {
			shaders.clear();
		}
	} else {
		ShaderType shaderType = GetShaderTypeFromFileName(shaderName);

		if (shaderType != ShaderType::Count) {
			shaders_[int(shaderType)].erase(shaderName);
		}
	}
}

ShaderType SHEngine::PSO::ShaderShelf::GetShaderTypeFromFileName(const std::string& fileName) {
	// "Name.VS.hlsl" → stem "Name.VS" → extension ".VS"
	fs::path path(fileName);
	if (path.extension() != ".hlsl") {
		return ShaderType::Count;
	}

	const std::string typeExtension = path.stem().extension().string();

	if (typeExtension == ".VS") {
		return ShaderType::VERTEX_SHADER;
	}
	if (typeExtension == ".PS") {
		return ShaderType::PIXEL_SHADER;
	}
	if (typeExtension == ".CS") {
		return ShaderType::COMPUTE_SHADER;
	}
	if (typeExtension == ".MS") {
		return ShaderType::MESH_SHADER;
	}
	if (typeExtension == ".AS") {
		return ShaderType::AMPLIFICATION_SHADER;
	}

	// 登録してない、または名前が間違っているShader
	return ShaderType::Count;
}
