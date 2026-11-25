#pragma once
#include <Render/PSO/ShelfManager.h>
#include <Core/Command/CommandObject.h>

namespace SHEngine::PSO {
	//CSPSOManagerはComputeShader用のPSOを管理するクラス
	//RootSignatureとShaderBytecodeはグラフィックスと共通のShelfManager(RootSignatureShelf / ShaderShelf)から取得し、
	//それらからPSOを生成してキャッシュする。

	class CSPSOManager {
	public:

		void Initialize(DXDevice* device, ShelfManager* shelfManager);
		
		// @brief ComputeShader用のRootSignatureとPSOをcmdListにセットする。
		// @param cmdList セットするコマンドリスト
		// @param rootConfig ルートシグネチャの設定。rootParamsの順番がそのままRootParameterIndexになる。shaderはCOMPUTE_SHADERであること。
		// @param computeShaderName 使用するComputeShaderのファイル名(Assets/Shader/Compute/以下)
		bool SetPSO(ID3D12GraphicsCommandList* cmdList, const RootSignatureConfig& rootConfig, const std::string& computeShaderName);

		// @brief ComputeShaderのリフレクション結果を取得する(未コンパイルならコンパイルする)。失敗時はnullptr
		// @param computeShaderName ComputeShaderのファイル名(Assets/Shader/Compute/以下)
		const ShaderReflectionData* GetReflection(const std::string& computeShaderName) const;

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
		ShelfManager* shelfManager_ = nullptr;

		// ComputeShaderはShaderShelf(Assets/Shader/)のCompute/以下に置かれている
		const std::string directory_ = "Compute/";

	};

}
