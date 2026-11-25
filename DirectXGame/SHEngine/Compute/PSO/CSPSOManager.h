#pragma once
#include <Compute/PSO/CSRootSignature.h>
#include <Compute/PSO/CSShaderShelf.h>
#include <Core/Command/CommandObject.h>

namespace SHEngine::PSO {
	//CSPSOManagerはComputeShader用のPSOを管理するクラス
	//RootSignatureとShaderBytecodeを管理するクラスを持ち、必要に応じてそれらからPSOを生成して返す
	//PSO自体はキャッシュせず、必要なときに都度生成する。理由はPSOの数が多くなりすぎるのを防ぐため。
	//RootSignatureとShaderBytecodeはPSOの生成に必要な情報であるため、CSPSOManagerが管理する。
	//CSPSOManagerはRootSignatureとShaderBytecodeを管理するクラスを持ち、必要に応じてそれらからPSOを生成して返す。

	class CSPSOManager {
	public:

		void Initialize(DXDevice* device, std::map<SamplerID, D3D12_STATIC_SAMPLER_DESC> samplers);
		
		// @brief ComputeShader用のRootSignatureとPSOをcmdListにセットする。
		// @param cmdList セットするコマンドリスト
		// @param rootConfig ルートシグネチャの設定。rootParamsの順番がそのままRootParameterIndexになる。
		// @param computeShaderName 使用するComputeShaderのファイル名
		bool SetPSO(ID3D12GraphicsCommandList* cmdList, const RootSignatureConfig& rootConfig, const std::string& computeShaderName);

	private:

		struct Config {
			RootSignatureConfig rootConfig;
			std::string computeShaderName;
			bool operator<(const Config& other) const {
				return std::tie(rootConfig, computeShaderName) < std::tie(other.rootConfig, other.computeShaderName);
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
