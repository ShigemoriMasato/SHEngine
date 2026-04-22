#pragma once
#include <Render/PSO/PSOManager.h>

namespace SHEngine::PSO {

	/**
	 * @class Editor
	 * @brief パイプラインステートオブジェクト（PSO）を編集・設定するクラス
	 *
	 * 現在設定されているPSOを変更し、PSOマネージャーを通して
	 * パイプラインステートをコマンドリストに設定する。
	 */
	class Editor {
	public:

		/**
		 * @brief コンストラクタ
		 * @param device DirectX12デバイス
		 */
		Editor();
		/// @brief デストラクタ
		~Editor() = default;

		/**
		 * @brief PSOエディタを初期化
		 * @param device DirectX12デバイス
		 */
		void Initialize(DXDevice* device);

		/**
		 * @brief フレーム開始時の初期化
		 * @param commandList コマンドリスト
		 */
		void FrameInitialize(ID3D12GraphicsCommandList* commandList);

		/**
		 * @brief 設定した内容でPSOを設定し、内部の設定をデフォルトに戻す
		 *
		 * デフォルト設定はEngine/PSO/PSOConfigに定義されている。
		 * @param commandList コマンドリスト
		 * @param config PSO設定
		 */
		void SetPSO(ID3D12GraphicsCommandList* commandList, const PSO::Config& config);

		/**
		 * @brief サンプラー記述子を取得
		 * @return サンプラー記述子のマップ
		 */
		std::map<SamplerID, D3D12_STATIC_SAMPLER_DESC> GetSamplers() const { return psoManager_->GetSamplers(); }

	private:

		/// @brief PSOマネージャー
		std::unique_ptr<Manager> psoManager_;

		/// @brief 現在のPSO設定
		PSO::Config nowConfig_{};

		/// @brief ログ出力用logger
		std::shared_ptr<spdlog::logger> logger_;

		/// @brief 初回フラグ
		bool isFirst_ = true;

	};

}
