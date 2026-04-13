#pragma once
#include <Compute/PSO/CSRootSignature.h>
#include <Compute/PSO/CSShaderShelf.h>
#include <Core/Command/CommandManager.h>

namespace SHEngine::PSO {
	//CSPSOManagerはComputeShader用のPSOを管理するクラス
	//RootSignatureとShaderBytecodeを管理するクラスを持ち、必要に応じてそれらからPSOを生成して返す
	//PSO自体はキャッシュせず、必要なときに都度生成する。理由はPSOの数が多くなりすぎるのを防ぐため。
	//RootSignatureとShaderBytecodeはPSOの生成に必要な情報であるため、CSPSOManagerが管理する。
	//CSPSOManagerはRootSignatureとShaderBytecodeを管理するクラスを持ち、必要に応じてそれらからPSOを生成して返す。

	class CSPSOManager {
	public:

		void Initialize(DXDevice* device);
		
		void SetPSO(CmdObj* cmdObj, int cbv, int srv, int uav, std::string computeShaderName);

	private:

		struct Config {
			int cbv;
			int srv;
			int uav;
			std::string computeShaderName;
			bool operator==(const Config& other) const {
				return cbv == other.cbv && srv == other.srv && uav == other.uav;
			}
			bool operator<(const Config& other) const {
				if (cbv != other.cbv) return cbv < other.cbv;
				if (srv != other.srv) return srv < other.srv;
				if (uav != other.uav) return uav < other.uav;
				return computeShaderName < other.computeShaderName;
			}
		};
		struct PSO {
			Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState;
		};
		std::map<Config, PSO> pso_;

		DXDevice* device_ = nullptr;

		std::unique_ptr<CSRootSignature> rootSignature_;
		std::unique_ptr<CSShaderShelf> shaderShelf_;

	};

}
