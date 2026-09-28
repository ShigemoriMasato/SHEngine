#include "CSShaderShelf.h"

using namespace SHEngine::PSO;

void CSShaderShelf::Initialize(DXDevice* device) {
    shaderBytecodeMap_.clear();
	device_ = device;
}

D3D12_SHADER_BYTECODE SHEngine::PSO::CSShaderShelf::GetShaderBytecode(const std::string& name) {
	const auto& it = shaderBytecodeMap_.find(name);
	if(it != shaderBytecodeMap_.end()) {
		return it->second;
	}
	else {
		IDxcBlob* blob = device_->CompileShader(directory_ + name, ShaderType::COMPUTE_SHADER);
		D3D12_SHADER_BYTECODE bytecode{};
		bytecode.pShaderBytecode = blob->GetBufferPointer();
		bytecode.BytecodeLength = blob->GetBufferSize();
		shaderBytecodeMap_[name] = bytecode;
		return bytecode;
	}

	return {};
}
