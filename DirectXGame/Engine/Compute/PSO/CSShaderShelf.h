#pragma once
#include <Core/DXDevice.h>

namespace SHEngine::PSO {

	class CSShaderShelf {
	public:

		void Initialize(DXDevice* device);

		D3D12_SHADER_BYTECODE GetShaderBytecode(const std::string& name);

	private:

		DXDevice* device_ = nullptr;
		std::unordered_map<std::string, D3D12_SHADER_BYTECODE> shaderBytecodeMap_;
		const std::string directory_ = "Assets/Shader/Compute/";
	};

}
