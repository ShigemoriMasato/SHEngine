#pragma once
#include <Core/DXDevice.h>
#include <vector>
#include <Tool/Binary/BinaryManager.h>
#include <Render/PSO/PSOConfig.h>

namespace SHEngine::PSO {

	/**
	 * @class Manager
	 * @brief パイプラインステートオブジェクト(PSO)を管理するクラス
	 *
	 * シェーダー、ブレンドステート、深度ステンシル、ラスタライザー、
	 * ルートシグネチャ、入力レイアウトなどの設定を統合してPSOを生成、管理する。
	 * 同じ設定のPSOはキャッシュされ、再利用される。
	 */
	class Manager {
	public:

		/**
		 * @brief PSOManagerのコンストラクタ
		 *
		 * @param device DirectX12デバイス
		 */
		Manager(DXDevice* device);

		/**
		 * @brief デストラクタ
		 */
		~Manager();

		/**
		 * @brief 初期化処理
		 *
		 * シェーダーのコンパイルや各種ステートの設定を行う。
		 */
		void Initialize();

		/**
		 * @brief PSOの取得
		 *
		 * 指定された設定にPSOを取得する。存在しない場合は新規作成する。
		 *
		 * @param config PSO設定
		 * @return パイプラインステートオブジェクト
		 */
		ID3D12PipelineState* GetPSO(const PSO::Config& config);

		/**
		 * @brief ルートシグネチャの取得
		 *
		 * @param config ルートシグネチャ設定
		 * @return ルートシグネチャ
		 */
		ID3D12RootSignature* GetRootSignature(const RootSignatureConfig& config) const;

		/**
		 * @brief シェーダーシェルフの取得
		 *
		 * @return シェーダーシェルフのポインタ
		 */
		ShaderShelf* GetShaderShelf() const { return shaderShelf_.get(); }

	private:

		/**
		 * @brief PSOの作成
		 *
		 * 指定された設定からPSOを作成し、内部マップに登録する。
		 *
		 * @param config PSOの設定
		 */
		void CreatePSO(PSO::Config config);

		/// @brief DirectX12デバイス
		ID3D12Device* device_ = nullptr;

		/// @brief PSOのキャッシュマップ(PSOConfig -> ID3D12PipelineState*)
		std::unordered_map<PSO::Config, ID3D12PipelineState*> psoMap_;

		/// @brief シェーダー管理
		std::unique_ptr<ShaderShelf> shaderShelf_{};
		/// @brief ブレンドステート管理
		std::unique_ptr<BlendStateShelf> blendStateShelf_{};
		/// @brief 深度ステンシル管理
		std::unique_ptr<DepthStencilShelf> depthStencilShelf_{};
		/// @brief ラスタライザー管理
		std::unique_ptr<RasterizerShelf> rasterizerShelf_{};
		/// @brief ルートシグネチャ管理
		std::unique_ptr<RootSignatureShelf> rootSignatureShelf_{};
		/// @brief 入力レイアウト管理
		std::unique_ptr<InputLayoutShelf> inputLayoutShelf_{};

		/// @brief バイナリデータ管理
		std::unique_ptr<BinaryManager> binaryManager_ = nullptr;
		/// @brief シェーダー編集データのファイル名
		const std::string shaderDataFile = "ShaderEditData.bin";

		/// @brief ログ出力用logger
		std::shared_ptr<spdlog::logger> logger_ = nullptr;

		/// @brief プリミティブトポロジとトポロジタイプの変換マップ
		static std::unordered_map<D3D12_PRIMITIVE_TOPOLOGY, D3D12_PRIMITIVE_TOPOLOGY_TYPE> topologyMap_;
	};

}
