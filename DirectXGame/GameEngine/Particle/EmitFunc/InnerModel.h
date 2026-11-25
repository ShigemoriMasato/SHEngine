#pragma once
#include <Particle/Emitter.h>
#include <Assets/Model/ModelData.h>
#include <cstdint>
#include <random>
#include <string>
#include <vector>

namespace Particle::InnerModel {

	/// @brief モデル内部の八分木を事前計算する際の設定
	/// @note 八分木の最大深さの上限は InnerModel.cpp の kMaxDepth で固定
	struct BuildDesc {
		// 八分木の最大深さの下限 (ローポリでも最低限これだけは分割する)
		uint32_t minDepth = 4;
		// 葉セルの目標サイズ = ポリゴンの平均辺長 * cellScale。小さいほど深く分割する
		float cellScale = 1.0f;
		// セル内の三角形がこの数以下になったら分割をやめて混在葉にする
		uint32_t leafTriangleCount = 8;
		// 使用スレッド数 (0 = hardware_concurrency)
		uint32_t threadCount = 0;
	};

	/// @brief 八分木の葉セル
	struct Cell {
		Vector3 center;
		float half;				// セルの半辺長
		uint32_t triOffset;		// InnerModelVolume::triangles の三角形番号 (mixed のときのみ有効)
		uint32_t triCount;		// このセルと重なる三角形の数 (mixed のときのみ有効)
		uint32_t flags;			// kCellFlag*
		int32_t centerWinding;	// セル中心のワインディング数 (0 以外なら内部。混在セルの判定の基準になる)

		static constexpr uint32_t kCellFlagMixed = 1u << 0;			// 内外混在セル (無ければ完全に内部)

		bool IsMixed() const { return (flags & kCellFlagMixed) != 0; }
	};
	static_assert(sizeof(Cell) == 32, "InnerModelCell は 32byte を想定");

	/// @brief 事前計算の結果 (ローカル空間)。出現位置はここから実行時に連続値でサンプリングする
	struct Volume {
		Vector3 min;							// モデルのAABB
		Vector3 max;
		std::vector<Cell> cells;		// 内部セル + 混在セル
		std::vector<Vector3> triangles;			// 混在セルの内外判定用の三角形 (3頂点ずつ、セルごとに連続)
		// 体積比でセルを O(1) で選ぶための Walker の Alias テーブル
		std::vector<float> aliasProbability;
		std::vector<uint32_t> aliasIndex;
	};

	/// @brief 事前計算の統計 (デバッグ用)
	struct DebugInfo {
		uint32_t triangleCount = 0;
		uint32_t maxDepth = 0;
		uint32_t insideLeafCount = 0;	// 完全に内部のセル数
		uint32_t mixedLeafCount = 0;	// 内外が混在したセル数
		double buildMilliseconds = 0.0;
	};

	/// @brief emitter のパーティクルをモデル内部に一様な乱数位置で配置する、一度だけ実行される処理を追加する
	/// @param container 処理で使うGPUBufferの作成先。処理が実行されるまで (最低でも数フレーム) 生存している必要がある
	/// @param emitCount 配置するパーティクル数。0以下、またはエミッターの数を超える場合はエミッターの全パーティクル
	/// @param modelData 配置先のモデル。八分木が未作成/古い場合はここで作成する (重い)
	/// @param scale 配置したパーティクルのスケール
	/// @param color 配置したパーティクルの色
	/// @return 処理を追加できたか (モデルの八分木が作れなかった場合は false)
	bool EmitInnerModel(Emitter* emitter, SHEngine::BufferContainer* container, int emitCount, const ModelData* modelData,
		float scale = 1.0f, const Vector4& color = { 1.0f, 1.0f, 1.0f, 1.0f });

	/// @brief 八分木を計算してバイナリに保存する
	/// @return 成功したか (メッシュが閉じていない/三角形がない等で失敗)
	bool CreateInnderModelBuffer(const ModelData* modelData, const BuildDesc& desc = {}, DebugInfo* stats = nullptr);

	/// @brief 八分木を計算する (保存はしない)
	bool BuildInnerModelVolume(const ModelData* modelData, const BuildDesc& desc, Volume& out, DebugInfo* stats = nullptr);

	/// @brief 保存済みの八分木を読み込む。ファイルが壊れている/モデルと一致しない場合は false
	bool LoadInnerModelBuffer(const ModelData* modelData, Volume& out);

	/// @brief 保存済みの八分木が存在し、現在のモデルと一致するか
	bool ExistsInnerModelBuffer(const ModelData* modelData);

	/// @brief モデル内部の一様な乱数位置を1つ求める (ローカル空間)
	/// @param maxAttempts 混在セルで外部を引いた場合の引き直し回数の上限
	/// @return 上限までに内部の点が得られなかった場合は false
	bool SampleInnerModel(const Volume& volume, std::mt19937_64& rng, Vector3& out, uint32_t maxAttempts = 64);

};
