#pragma once
#include <d3d12.h>
#include <dxcapi.h>
#include <string>
#include <vector>
#include <array>
#include <wrl.h>
#include <Logger/Logger.h>
#include <unordered_map>
#include <filesystem>

enum class ShaderType {
	VERTEX_SHADER,
	PIXEL_SHADER,

	Count
};

class ShaderShelf {
public:

	ShaderShelf();
	~ShaderShelf();

	void CompileAllShader();

	std::list<D3D12_SHADER_BYTECODE> GetShaderBytecodes(ShaderType shaderType);

	std::list<std::string> GetShaderNames(ShaderType shaderType);

	D3D12_SHADER_BYTECODE GetShaderBytecode(ShaderType shaderType, std::string shaderName);

private:

	void RegistShaderByteCode(std::wstring shaderName, ShaderType shaderType);

	Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils = nullptr;
	Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler = nullptr;
	Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler = nullptr;

	std::filesystem::path basePath_ = "Assets/Shader";

	std::array<std::wstring, static_cast<size_t>(ShaderType::Count)> compileVersions_;

	std::list<IDxcBlob*> shaderBlobs_;

	std::array<std::unordered_map<std::string, D3D12_SHADER_BYTECODE>, static_cast<size_t>(ShaderType::Count)> shaderBytecodes_;

	std::shared_ptr<spdlog::logger> logger_ = nullptr;
};
