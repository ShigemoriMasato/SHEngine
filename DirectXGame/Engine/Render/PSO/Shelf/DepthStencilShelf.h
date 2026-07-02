#pragma once
#include <cstdint>
#include <vector>
#include <d3d12.h>

namespace SHEngine::PSO {

	/**
	 * @enum DepthStencilID
	 * @brief 深度ステンシルモードのID
	 */
	enum class DepthStencilID : uint8_t {
		None,			///< 深度テストと深度ステンシルを使用しない
		Default,		///< 不透明オブジェクト用(深度書き込み有効)
		Transparent,	///< 透明オブジェクト用(深度書き込み無効、描画順に注意)

		Count           ///< 深度ステンシルモードの総数
	};

	/**
	 * @class DepthStencilShelf
	 * @brief 深度ステンシルステートを管理するクラス
	 *
	 * 複数の深度ステンシルモードを事前に定義し、IDで取得できるようにする。
	 */
	class DepthStencilShelf {
	public:

		/**
		 * @brief コンストラクタ
		 *
		 * 各種深度ステンシルステートを初期化する。
		 */
		DepthStencilShelf();

		/**
		 * @brief デストラクタ
		 */
		~DepthStencilShelf();

		/**
		 * @brief 深度ステンシル設定の取得
		 *
		 * @param id 深度ステンシルID
		 * @return 深度ステンシルの設定
		 */
		D3D12_DEPTH_STENCIL_DESC GetDepthStencilDesc(DepthStencilID id) const;

	private:

		/// @brief 事前定義された深度ステンシル設定の配列
		std::vector<D3D12_DEPTH_STENCIL_DESC> depthStencilDescs_;

	};

} // namespace SHEngine

