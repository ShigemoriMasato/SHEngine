#pragma once
#include "Shelf/ShaderShelf.h"
#include "Shelf/BlendStateShelf.h"
#include "Shelf/DepthStencilShelf.h"
#include "Shelf/RasterizerShelf.h"
#include "Shelf/RootSignatureShelf.h"
#include "Shelf/InputLayoutShelf.h"

namespace SHEngine::PSO {

	enum class Topology : uint8_t {
		Triangle,
		Line,
		Point,

		Count
	};

	/**
	 * @struct PSOConfig
	 * @brief パイプラインステートオブジェクト（PSO）の設定をまとめた構造体
	 *
	 * シェーダー、ブレンドステート、深度ステンシル、ラスタライザー、
	 * ルートシグネチャ、入力レイアウトなどの描画パイプライン設定を保持する。
	 */
	struct Config {
		/// @brief 頂点シェーダーファイル名
		std::string vs = ".VS.hlsl";
		/// @brief ピクセルシェーダーファイル名
		std::string ps = "Object3d.PS.hlsl";
		/// @brief ルートシグネチャ設定
		RootSignatureConfig rootConfig = {};
		/// @brief 入力レイアウトID
		InputLayoutID inputLayoutID = InputLayoutID::Default;
		/// @brief ブレンドステートID
		BlendStateID blendID[8] = {};
		/// @brief 深度ステンシルID
		DepthStencilID depthStencilID = DepthStencilID::Default;
		/// @brief ラスタライザーID
		RasterizerID rasterizerID = RasterizerID::Fill;
		/// @brief プリミティブトポロジー
		Topology topology = Topology::Triangle;
		/// @brief RTVのフォーマット
		DXGI_FORMAT rtvFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		/// @brief RTVの数
		UINT rtvNum = 1;
		/// @brief DSVが必要かどうか
		bool isDSV = true;
		/// @brief DSVのフォーマット
		DXGI_FORMAT dsvFormat = DXGI_FORMAT_D32_FLOAT;

		/**
		* @brief 等価演算子
		* @param other 比較対象
		* @return 全てのメンバが等しい場合true
		*/
		bool operator==(const Config& other) const {
			return vs == other.vs &&
				ps == other.ps &&
				blendID[0] == other.blendID[0] &&
				blendID[1] == other.blendID[1] &&
				blendID[2] == other.blendID[2] &&
				blendID[3] == other.blendID[3] &&
				blendID[4] == other.blendID[4] &&
				blendID[5] == other.blendID[5] &&
				blendID[6] == other.blendID[6] &&
				blendID[7] == other.blendID[7] &&
				depthStencilID == other.depthStencilID &&
				rasterizerID == other.rasterizerID &&
				rootConfig == other.rootConfig &&
				inputLayoutID == other.inputLayoutID &&
				topology == other.topology &&
				rtvFormat == other.rtvFormat &&
				rtvNum == other.rtvNum &&
				isDSV == other.isDSV &&
				dsvFormat == other.dsvFormat;
		}

		/**
		 * @brief 不等価演算子
		 * @param other 比較対象
		 * @return メンバが異なる場合true
		 */
		bool operator!=(const Config& other) const {
			return !(*this == other);
		}
	};

} // namespace SHEngine

/**
 * @brief std::hash特殊化（PSOConfigをunordered_mapのキーとして使用可能にする）
 */
namespace std {
	template<>
	struct hash<SHEngine::PSO::Config> {
		/**
		 * @brief ハッシュ値計算演算子
		 * @param cfg PSOConfig設定
		 * @return ハッシュ値
		 */
		size_t operator()(const SHEngine::PSO::Config& cfg) const {
			size_t h = 0;
			// 文字列
			hash_combine(h, std::hash<std::string>()(cfg.vs));
			hash_combine(h, std::hash<std::string>()(cfg.ps));
			// ルートシグネチャ設定（既にハッシュ特殊化あり）
			hash_combine(h, std::hash<SHEngine::PSO::RootSignatureConfig>()(cfg.rootConfig));
			// 入力レイアウト（enum -> 整数）
			hash_combine(h, std::hash<uint8_t>()(static_cast<uint8_t>(cfg.inputLayoutID)));
			// ブレンドステート配列
			for (const auto& b : cfg.blendID) {
				hash_combine(h, std::hash<uint8_t>()(static_cast<uint8_t>(b)));
			}
			// 深度ステンシル / ラスタライザ / トポロジ / フォーマット類
			hash_combine(h, std::hash<uint8_t>()(static_cast<uint8_t>(cfg.depthStencilID)));
			hash_combine(h, std::hash<uint8_t>()(static_cast<uint8_t>(cfg.rasterizerID)));
			hash_combine(h, std::hash<UINT>()(static_cast<UINT>(cfg.topology)));
			hash_combine(h, std::hash<UINT>()(static_cast<UINT>(cfg.rtvFormat)));
			hash_combine(h, std::hash<UINT>()(static_cast<UINT>(cfg.rtvNum)));
			hash_combine(h, std::hash<bool>()(cfg.isDSV));
			hash_combine(h, std::hash<UINT>()(static_cast<UINT>(cfg.dsvFormat)));
			return h;
		}

	private:
		/**
		 * @brief ハッシュ値を結合する（Boost風実装）
		 * @param seed 元のハッシュ値（結果もここに格納）
		 * @param value 結合するハッシュ値
		 */
		static void hash_combine(size_t& seed, size_t value) {
			seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		}
	};
}