#pragma once
#include <vector>
#include <cstdint>
#include <d3d12.h>
#include <wrl.h>
#include <Tool/Logger/Logger.h>
#include "ShaderShelf.h"

enum class BufferType : uint8_t {
	CBV = 1 << 0,
	SRV = 1 << 1,
	UAV = 1 << 2,

	CBV_SRV = 0b011,
	//CBV_UAV = 0b101,		使えないやつ
	SRV_UAV = 0b110,
	//CBV_SRV_UAV = 0b111,	使えないやつ

	ReadBack = 0b10000,
};

uint8_t operator&(uint8_t a, BufferType b);
uint8_t operator~(BufferType a);

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
		Point = MinNearestMipmapNearest,	///< 可読性向上用

		MinLinearMipmapNearest = 1 << 10,  ///< 縮小時リニア+ミップマップニアレスト
		MinNearestMipmapLinear = 1 << 11,  ///< 縮小時ニアレスト+ミップマップリニア
		MinLinearMipmapLinear = 1 << 12,   ///< 縮小時リニア+ミップマップリニア

		ClampClamp_MinMagNearest = 1 << 13,  ///< S座標・T座標共に
	};

	uint32_t operator|(SamplerID a, SamplerID b);
	uint32_t operator|(uint32_t a, SamplerID b);
	bool operator<(SamplerID a, SamplerID b);

	struct RootParam {
		ShaderType shader;
		int registerNumber;
		BufferType bufferType;
	};

	/**
	 * @struct RootSignatureConfig
	 * @brief ルートシグネチャの設定情報
	 *
	 * ルートシグネチャに必要な定数バッファ、シェーダーリソース、
	 * テクスチャ、サンプラーの設定をまとめた構造体。
	 */
	struct RootSignatureConfig {
		std::vector<RootParam> rootParams;                      ///< ルートパラメータのリスト

		bool useTexture = false;                                ///< テクスチャ配列を使用するか
		uint32_t samplers = uint32_t(SamplerID::Default);        ///< サンプラーIDのビットマスク

		bool operator<(const RootSignatureConfig& other) const;
		bool operator==(const RootSignatureConfig& other) const;
	};

} // namespace SHEngine

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

		RootSignatureShelf(ID3D12Device2* device);

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

		ID3D12Device2* device_ = nullptr;  ///< D3D12デバイスポインタ

		std::map<RootSignatureConfig, ID3D12RootSignature*> rootSignatureMap_;  ///< ルートシグネチャのキャッシュマップ
		std::map<SamplerID, D3D12_STATIC_SAMPLER_DESC> samplers_;  ///< サンプラー記述子のマップ

		std::shared_ptr<spdlog::logger> logger_ = nullptr;  ///< ロガー

	};

} // namespace SHEngine





// unordered_mapで使用するハッシュ関数たち
namespace std {
	template<>
	struct hash<SHEngine::PSO::RootParam> {
		size_t operator()(const SHEngine::PSO::RootParam& param) const {
			size_t h = 0;
			hash_combine(h, hash<int>()(param.registerNumber));
			hash_combine(h, hash<uint8_t>()(uint8_t(param.bufferType)));
			return h;
		}
	private:
		static void hash_combine(size_t& seed, size_t value) {
			seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		}
	};

	template<>
	struct hash<SHEngine::PSO::RootSignatureConfig> {
		size_t operator()(const SHEngine::PSO::RootSignatureConfig& cfg) const {
			size_t h = 0;
			for (const auto& param : cfg.rootParams) {
				hash_combine(h, hash<SHEngine::PSO::RootParam>()(param));
			}
			hash_combine(h, hash<bool>()(cfg.useTexture));
			hash_combine(h, hash<uint32_t>()(cfg.samplers));
			return h;
		}
	private:
		static void hash_combine(size_t& seed, size_t value) {
			seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		}
	};
}
