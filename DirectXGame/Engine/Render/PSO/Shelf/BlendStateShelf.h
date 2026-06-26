#pragma once
#include <cstdint>
#include <d3d12.h>
#include <vector>

namespace SHEngine::PSO {

	/**
	 * @enum BlendStateID
	 * @brief ブレンドモードのID
	 */
	enum class BlendStateID : uint8_t {
		Normal,     ///< 通常ブレンド(α合成)
		Add,        ///< 加算ブレンド
		Subtract,   ///< 減算ブレンド
		Multiply,   ///< 乗算ブレンド
		Screen,     ///< スクリーンブレンド
		None,       ///< ブレンドなし

		Count       ///< ブレンドモードの総数
	};

	/**
	 * @class BlendStateShelf
	 * @brief ブレンドステートを管理するクラス
	 *
	 * 複数のブレンドモードを事前に定義し、IDで取得できるようにする。
	 */
	class BlendStateShelf {
	public:

		BlendStateShelf();
		~BlendStateShelf();

		/**
		 * @brief ブレンドステートの取得
		 *
		 * @param id ブレンドステートID
		 * @return ブレンドステートの設定
		 */
		D3D12_BLEND_DESC GetBlendState(const BlendStateID* id) const;

	private:

		/// @brief 事前定義されたブレンドステートの配列
		std::vector<D3D12_RENDER_TARGET_BLEND_DESC> blendStates_;

	};

} // namespace SHEngine
