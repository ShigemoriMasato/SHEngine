#pragma once
#include "PSOConfig.h"

namespace SHEngine::PSO {

	struct ConfigMSType {
		std::string ms = "Simple.MS.hlsl";
		std::string ps = "White.PS.hlsl";

		BlendStateID blendID[8] = {};
		RootSignatureConfig rootConfig = {};
		RasterizerID rasterizerID = RasterizerID::Fill;
		Topology topology = Topology::Triangle;
		DepthStencilID depthStencilID = DepthStencilID::Default;
		uint32_t rtvNum = 1;
		
	};

	// MeshShaderを使用するPSOを管理するクラス
	class ManagerMSType {
	public:

		ManagerMSType();

		void Initialize();

	private:

		

	};

}
