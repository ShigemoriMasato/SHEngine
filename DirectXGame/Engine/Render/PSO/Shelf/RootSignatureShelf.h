#pragma once
#include <vector>
#include <cstdint>
#include <d3d12.h>
#include <wrl.h>
#include <Tool/Logger/Logger.h>
#include "ShaderShelf.h"

namespace SHEngine::PSO {

	/**
	 * @enum SamplerID
	 * @brief サンプラーID(ビットフラグで複数指定可能)
	 */
	enum class SamplerID : uint32_t {
		Non = 0,             ///< サンプラーなし
		Default = 1 << 0,     ///< リピートサンプラー
		ClampT = 1 << 1,     ///< T座標をクランプ
		MirrorT = 1 << 2,    ///< T座標をミラー
		ClampS = 1 << 3,     ///< S座標をクランプ
		MirrorS = 1 << 4,    ///< S座標をミラー

		MagNearest = 1 << 5,  ///< 拡大時ニアレストネイバー補間
		MagLinear = 1 << 6,   ///< 拡大時リニア補間

		MinNearest = 1 << 7,             ///< 縮小時ニアレストネイバー補間
		MinLinear = 1 << 8,              ///< 縮小時リニア補間
		MinNearestMipmapNearest = 1 << 9,  ///< 縮小時ニアレスト+ミップマップニアレスト
		MinLinearMipmapNearest = 1 << 10,  ///< 縮小時リニア+ミップマップニアレスト
		MinNearestMipmapLinear = 1 << 11,  ///< 縮小時ニアレスト+ミップマップリニア
		MinLinearMipmapLinear = 1 << 12,   ///< 縮小時リニア+ミップマップリニア

		ClampClamp_MinMagNearest = 1 << 13,  ///< S座標・T座標共に
	};

	/**
	 * @brief SamplerIDのビットOR演算子
	 * @param a 左オペランド
	 * @param b 右オペランド
	 * @return ビットORの結果
	 */
	uint32_t operator|(SamplerID a, SamplerID b);

	/**
	 * @brief uint32_tとSamplerIDのビットOR演算子
	 * @param a 左オペランド（uint32_t型）
	 * @param b 右オペランド（SamplerID型）
	 * @return ビットORの結果
	 */
	uint32_t operator|(uint32_t a, SamplerID b);

	/**
	 * @brief SamplerIDの比較演算子
	 * @param a 左オペランド
	 * @param b 右オペランド
	 * @return a < bの結果
	 */
	bool operator<(SamplerID a, SamplerID b);

	/**
	 * @struct RootSignatureConfig
	 * @brief ルートシグネチャの設定情報
	 *
	 * ルートシグネチャに必要な定数バッファ、シェーダーリソース、
	 * テクスチャ、サンプラーの設定をまとめた構造体。
	 */
	struct RootSignatureConfig {
		std::pair<int, int> cbvNums{};                         ///< 定数バッファ数<Vertex, Pixel>
		std::pair<int, int> srvNums{};                         ///< シェーダーリソース数<Vertex, Pixel>（上限8）
		std::pair<int, int> uavNums{};
		std::pair<int, int> textureNums{};
		std::pair<int, int> ddsNums{};
		bool useTexture = false;                                ///< テクスチャ配列を使用するか
		uint32_t samplers = uint32_t(SamplerID::Default);        ///< サンプラーIDのビットマスク

		/**
		 * @brief 比較演算子（less than）
		 * @param other 比較対象
		 * @return この設定が他の設定より小さい場合true
		 */
		bool operator<(const RootSignatureConfig& other) const;

		/**
		 * @brief 等価演算子
		 * @param other 比較対象
		 * @return この設定が他の設定と等しい場合true
		 */
		bool operator==(const RootSignatureConfig& other) const;
	};

} // namespace SHEngine

namespace std {
	/**
	* @brief RootSignatureConfigのハッシュ関数特殊化
	*
	* RootSignatureConfigをunordered_mapなどのキーとして使用できるように
	* std::hashを特殊化しています。
	*/
	template<>
	struct hash<SHEngine::PSO::RootSignatureConfig> {
		/**
		* @brief ハッシュ値を計算
		* @param cfg ハッシュ計算対象の設定
		* @return 計算されたハッシュ値
		*/
		size_t operator()(const SHEngine::PSO::RootSignatureConfig& cfg) const {
			size_t h = 0;
			hash_combine(h, hash<int>()(cfg.cbvNums.first));
			hash_combine(h, hash<int>()(cfg.cbvNums.second));
			hash_combine(h, hash<int>()(cfg.srvNums.first));
			hash_combine(h, hash<int>()(cfg.srvNums.second));
			hash_combine(h, hash<bool>()(cfg.useTexture));
			hash_combine(h, hash<uint32_t>()(cfg.samplers));
			return h;
		}
	private:
		/**
		 * @brief ハッシュ値を結合
		 * @param seed 既存のハッシュ値
		 * @param value 結合する値
		 */
		static void hash_combine(size_t& seed, size_t value) {
			seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		}
	};
}

namespace SHEngine::PSO {
	/**
	 * @class RootSignatureShelf
	 * @brief ルートシグネチャの管理クラス
	 *
	 * 設定に基づいてルートシグネチャを作成・キャッシュし、
	 * 同じ設定の場合は既存のルートシグネチャを再利用します。
	 */
	class RootSignatureShelf {
	public:

		/**
		 * @brief コンストラクタ
		 * @param device D3D12デバイスポインタ
		 */
		RootSignatureShelf(ID3D12Device* device);

		/**
		 * @brief デストラクタ
		 */
		~RootSignatureShelf();

		/**
		 * @brief 設定に基づいてルートシグネチャを取得
		 *
		 * 既に同じ設定のルートシグネチャが存在する場合はそれを返し、
		 * 存在しない場合は新規作成してキャッシュします。
		 *
		 * @param config ルートシグネチャの設定
		 * @return ルートシグネチャのポインタ
		 */
		ID3D12RootSignature* GetRootSignature(const RootSignatureConfig& config);

		/**
		 * @brief サンプラー記述子を取得
		 * @return サンプラー記述子のマップ
		 */
		std::map<SamplerID, D3D12_STATIC_SAMPLER_DESC> GetSamplers() const { return samplers_; }

	private:

		/**
		 * @brief ルートシグネチャを作成
		 * @param config ルートシグネチャの設定
		 * @return 作成されたルートシグネチャのポインタ
		 */
		ID3D12RootSignature* CreateRootSignature(const RootSignatureConfig& config);

		ID3D12Device* device_ = nullptr;  ///< D3D12デバイスポインタ

		std::map<RootSignatureConfig, ID3D12RootSignature*> rootSignatureMap_;  ///< ルートシグネチャのキャッシュマップ
		std::map<SamplerID, D3D12_STATIC_SAMPLER_DESC> samplers_;  ///< サンプラー記述子のマップ

		std::shared_ptr<spdlog::logger> logger_ = nullptr;  ///< ロガー

	};

} // namespace SHEngine
