#pragma once
#include <Particle/Emitter.h>
#include <Assets/Model/ModelData.h>
#include <cstdint>
#include <string>
#include <vector>

namespace Particle {

	/// @brief モデル内部の発生候補点を事前計算する際の設定
	struct InnerModelBuildDesc {
		// 作成する候補点の数
		uint32_t pointCount = 10'000'000;
		// 八分木の最大深さの上限 (10 => 最長辺1024分割)
		uint32_t maxDepthLimit = 10;
		// 八分木の最大深さの下限 (ローポリでも最低限これだけは分割する)
		uint32_t minDepth = 4;
		// 葉セルの目標サイズ = ポリゴンの平均辺長 * cellScale。小さいほど深く分割する
		float cellScale = 1.0f;
		// セル内の三角形がこの数以下になったら分割をやめて混在葉にする
		uint32_t leafTriangleCount = 8;
		// 乱数シード
		uint64_t seed = 0x5EED5EEDull;
		// 使用スレッド数 (0 = hardware_concurrency)
		uint32_t threadCount = 0;
	};

	/// @brief 事前計算の結果 (ローカル空間)
	struct InnerModelPoints {
		Vector3 min;					// モデルのAABB
		Vector3 max;
		std::vector<Vector3> points;	// モデル内部の候補点
	};

	/// @brief 事前計算の統計 (デバッグ用)
	struct InnerModelBuildStats {
		uint32_t triangleCount = 0;
		uint32_t maxDepth = 0;
		uint32_t insideLeafCount = 0;	// 完全に内部のセル数
		uint32_t mixedLeafCount = 0;	// 内外が混在したセル数
		uint64_t rejectedCount = 0;		// 混在セルで外部と判定され棄却した点の数
		double buildMilliseconds = 0.0;
	};

	void EmitInnnerModel(Emitter* emitter, const Matrix4x4& transform, int emitCount, const ModelData* modelData);

	/// @brief 内部候補点を計算してバイナリに保存する
	/// @return 成功したか (メッシュが閉じていない/三角形がない等で失敗)
	bool CreateInnderModelBuffer(const ModelData* modelData, const InnerModelBuildDesc& desc = {}, InnerModelBuildStats* stats = nullptr);

	/// @brief 内部候補点を計算する (保存はしない)
	bool BuildInnerModelPoints(const ModelData* modelData, const InnerModelBuildDesc& desc, InnerModelPoints& out, InnerModelBuildStats* stats = nullptr);

	/// @brief 保存済みの内部候補点を読み込む。ファイルが壊れている/モデルと一致しない場合は false
	bool LoadInnerModelBuffer(const ModelData* modelData, InnerModelPoints& out);

	/// @brief 保存済みの内部候補点が存在し、現在のモデルと一致するか
	bool ExistsInnerModelBuffer(const ModelData* modelData);

};
