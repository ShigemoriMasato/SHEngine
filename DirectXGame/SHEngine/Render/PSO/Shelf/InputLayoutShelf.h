#pragma once
#include <vector>
#include <cstdint>
#include <d3d12.h>
#include <map>

namespace SHEngine {

	enum class VertexType {
		Position = 1 << 0,
		Texcoord = 1 << 1,
		Normal = 1 << 2,
		Color = 1 << 3,
		Influence = 1 << 4,

		Default = Position | Texcoord | Normal,
		Skinning = Position | Texcoord | Normal | Influence,

		PostEffect = Position | Texcoord,

		All = Position | Texcoord | Normal | Color | Influence,

		Count = 5
	};

}

namespace SHEngine::PSO {

	/**
	 * @class InputLayoutShelf
	 * @brief 入力レイアウトを管理するクラス
	 *
	 * 複数の入力レイアウトを事前に定義し、IDで取得できるようにする。
	 */
	class InputLayoutShelf {
	public:

		InputLayoutShelf();
		~InputLayoutShelf();

		/**
		 * @brief 入力レイアウト記述の取得
		 *
		 * @param id 入力レイアウトID
		 * @return 入力レイアウト記述
		 */
		D3D12_INPUT_ELEMENT_DESC GetInputLayoutDesc(uint32_t vertexType) const;

		std::vector<D3D12_INPUT_ELEMENT_DESC> GetInfluenceDesc() const { return influenceDesc_; }

	private:

		/// @brief 入力要素記述の配列(各レイアウト用)
		std::map<uint32_t, D3D12_INPUT_ELEMENT_DESC> inputElementDescs_;
		std::vector<D3D12_INPUT_ELEMENT_DESC> influenceDesc_;
		/// @brief 入力レイアウト記述の配列
		std::vector<D3D12_INPUT_LAYOUT_DESC> inputLayouts_;

	};

} // namespace SHEngine

