#include "InnerModel.h"

// BinaryManager の std::vector 登録処理 (RegisterValue<std::vector<T>>) に size_t -> uint32_t の暗黙変換警告(C4267)があり、
// 警告をエラーとして扱う設定ではインスタンス化した時点でビルドが通らないため、ここでのみ抑制する。
// (保存する配列は uint32_t に収まる大きさなので、変換による値の欠落は起きない)
#pragma warning(push)
#pragma warning(disable : 4267)
#include <Tool/Binary/BinaryManager.h>

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cfloat>
#include <thread>

/*
 * モデル内部の八分木の事前計算
 *
 * 1. モデルAABBの最長辺を1辺とする立方体をルートにした八分木を作る
 * 2. 各セルについて「重なる三角形」を親セルのリストから絞り込む (Triangle-AABB の分離軸判定)
 *    - 重なる三角形が無い     : セル全体が内部 or 外部。中心点の内外がそのままセルの状態
 *    - 重なる三角形がある     : 内外混在。最大深さ/三角形数の条件に達するまで8分割
 * 3. セル中心のワインディング数は、親の中心 -> 子の中心 の線分が「子セルの三角形」を横切った符号付きの回数を足して引き継ぐ
 *    (表に入る向きで +1、裏から出る向きで -1。線分は子セル内に収まるため、全ポリゴンを調べる必要が無い。
 *     全ポリゴンへの半直線判定はルートの1回のみ)
 *    ワインディング数が 0 以外なら内部 (非ゼロ規則)。偶奇規則と違い、メッシュ同士が重なった部分や
 *    内側に埋まった別メッシュも内部として扱える (面の向きが外向きに揃っていることが前提)
 * 4. 内部セル/混在セル (と混在セルに重なる三角形) と、体積比でセルを選ぶ Alias テーブルを保存する。点は保存しない
 *
 * 実行時 (SampleInnerModel)
 * 5. Alias テーブルで体積比に従ってセルを O(1) で選び、セル内に一様乱数で点を打つ。
 *    混在セルでは セル中心 -> 点 の線分で同様にワインディング数を求め、外部ならセルの選択からやり直す (内部体積に対して一様になる)
 */

namespace {
	const std::string ignoreString = "Assets/";
	// BinaryManager の basePath (Assets/Binary/) からの相対パス
	const std::string bufferDirectory = "Particle/InnerModel/";

	const std::string kSignature = "InnerModelOctree";
	// 3: 内外判定を偶奇からワインディング数に変更
	constexpr uint32_t kFormatVersion = 3;

	// 八分木の最大分割数 (最大深さ)。10 => 最長辺を 1024 分割
	constexpr uint32_t kMaxDepth = 12;

	// ---------------------------------------------------------------------------------
	// 軽量ベクトル (ホットループ用にインライン化)
	// ---------------------------------------------------------------------------------
	struct V3 {
		float x, y, z;
	};
	inline V3 operator+(const V3& a, const V3& b) { return { a.x + b.x, a.y + b.y, a.z + b.z }; }
	inline V3 operator-(const V3& a, const V3& b) { return { a.x - b.x, a.y - b.y, a.z - b.z }; }
	inline V3 operator*(const V3& a, float s) { return { a.x * s, a.y * s, a.z * s }; }
	inline float Dot(const V3& a, const V3& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
	inline V3 Cross(const V3& a, const V3& b) {
		return { a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x };
	}
	inline float Length(const V3& a) { return std::sqrt(Dot(a, a)); }

	inline V3 ToV3(const Vector3& v) { return { v.x, v.y, v.z }; }
	inline Vector3 ToVector3(const V3& v) { return { v.x, v.y, v.z }; }

	struct Tri {
		V3 v0, v1, v2;
	};

	// ---------------------------------------------------------------------------------
	// 交差判定
	// ---------------------------------------------------------------------------------

	/// Möller–Trumbore。o + t*d (0 < t <= tMax) と三角形の交差を符号付きで返す
	/// @return 交差しない: 0 / 表側から入る (d と法線が逆向き): +1 / 裏側から出る (d と法線が同じ向き): -1
	/// @note 法線は (v1 - v0) x (v2 - v0)。det = dot(e1, d x e2) = -dot(d, 法線) なので det > 0 が「入る」
	inline int IntersectTriangle(const V3& o, const V3& d, const Tri& tri, float tMax) {
		const V3 e1 = tri.v1 - tri.v0;
		const V3 e2 = tri.v2 - tri.v0;
		const V3 p = Cross(d, e2);
		const float det = Dot(e1, p);
		if (std::fabs(det) < 1e-20f) {
			return 0;
		}
		const float inv = 1.0f / det;
		const V3 s = o - tri.v0;
		const float u = Dot(s, p) * inv;
		if (u < 0.0f || u > 1.0f) {
			return 0;
		}
		const V3 q = Cross(s, e1);
		const float v = Dot(d, q) * inv;
		if (v < 0.0f || u + v > 1.0f) {
			return 0;
		}
		const float t = Dot(e2, q) * inv;
		if (t <= 0.0f || t > tMax) {
			return 0;
		}
		return det > 0.0f ? 1 : -1;
	}

	/// 線分 a -> b で変化するワインディング数 (winding(b) = winding(a) + 戻り値)
	inline int SegmentWinding(const V3& a, const V3& b, const std::vector<Tri>& tris, const uint32_t* indices, size_t count) {
		const V3 d = b - a;
		int winding = 0;
		for (size_t i = 0; i < count; ++i) {
			winding += IntersectTriangle(a, d, tris[indices[i]], 1.0f);
		}
		return winding;
	}

	/// 線分 a -> b で変化するワインディング数 (保存済みの三角形 = 3頂点ずつ並んだ配列版)
	inline int SegmentWinding(const V3& a, const V3& b, const Vector3* triangles, uint32_t count) {
		const V3 d = b - a;
		int winding = 0;
		for (uint32_t i = 0; i < count; ++i) {
			const Tri tri{ ToV3(triangles[i * 3]), ToV3(triangles[i * 3 + 1]), ToV3(triangles[i * 3 + 2]) };
			winding += IntersectTriangle(a, d, tri, 1.0f);
		}
		return winding;
	}

	/// 全三角形に対する半直線でワインディング数を求める。辺・頂点に当たった場合の誤判定を避けるため3方向の多数決
	/// 無限遠のワインディング数は 0 なので、winding(p) = -(p -> 無限遠 で変化した量)
	int WindingByRay(const V3& p, const std::vector<Tri>& tris) {
		const V3 dirs[3] = {
			{ 1.0f, 0.00131f, 0.00077f },
			{ -0.00113f, 1.0f, 0.00191f },
			{ 0.00173f, -0.00097f, 1.0f },
		};
		int results[3] = {};
		for (int i = 0; i < 3; ++i) {
			int winding = 0;
			for (const Tri& tri : tris) {
				winding += IntersectTriangle(p, dirs[i], tri, FLT_MAX);
			}
			results[i] = -winding;
		}
		// 2方向以上で一致した値を採用 (全て食い違った場合は1方向目)
		if (results[1] == results[2]) {
			return results[1];
		}
		return results[0];
	}

	/// Akenine-Möller の Triangle-AABB 分離軸判定 (center, half の箱)
	bool TriangleBoxOverlap(const V3& center, float half, const Tri& tri) {
		const V3 v0 = tri.v0 - center;
		const V3 v1 = tri.v1 - center;
		const V3 v2 = tri.v2 - center;

		// AABB の3軸
		auto outsideAxis = [half](float a, float b, float c) {
			return std::min({ a, b, c }) > half || std::max({ a, b, c }) < -half;
		};
		if (outsideAxis(v0.x, v1.x, v2.x)) { return false; }
		if (outsideAxis(v0.y, v1.y, v2.y)) { return false; }
		if (outsideAxis(v0.z, v1.z, v2.z)) { return false; }

		const V3 e0 = v1 - v0;
		const V3 e1 = v2 - v1;
		const V3 e2 = v0 - v2;

		// 三角形の平面
		const V3 n = Cross(e0, e1);
		const float r = half * (std::fabs(n.x) + std::fabs(n.y) + std::fabs(n.z));
		if (std::fabs(Dot(n, v0)) > r) {
			return false;
		}

		// 箱の軸 × 三角形の辺 の9軸
		auto separated = [&](const V3& axis) {
			const float p0 = Dot(axis, v0);
			const float p1 = Dot(axis, v1);
			const float p2 = Dot(axis, v2);
			const float rad = half * (std::fabs(axis.x) + std::fabs(axis.y) + std::fabs(axis.z));
			return std::min({ p0, p1, p2 }) > rad || std::max({ p0, p1, p2 }) < -rad;
		};
		for (const V3& e : { e0, e1, e2 }) {
			if (separated({ 0.0f, -e.z, e.y })) { return false; }
			if (separated({ e.z, 0.0f, -e.x })) { return false; }
			if (separated({ -e.y, e.x, 0.0f })) { return false; }
		}
		return true;
	}

	// ---------------------------------------------------------------------------------
	// 八分木
	// ---------------------------------------------------------------------------------
	struct BuildNode {
		V3 center;
		float half;
		uint32_t depth;
		int centerWinding;			// セル中心のワインディング数 (0 以外なら内部)
		std::vector<uint32_t> tris;	// このセルと重なる三角形
	};

	struct Leaf {
		V3 center;
		float half;
		bool mixed;			// false なら完全に内部
		int centerWinding;	// 中心点のワインディング数
		uint32_t triOffset;
		uint32_t triCount;
	};

	struct LeafList {
		std::vector<Leaf> leaves;
		std::vector<uint32_t> triIndices;
	};

	struct BuildContext {
		const std::vector<Tri>* tris;
		uint32_t maxDepth;
		uint32_t leafTriangleCount;
		// 判定を保守的にするための箱の拡大率 (重なり漏れは誤判定に直結するが、過検出は分割が増えるだけ)
		float boxExpand;
	};

	void Subdivide(const BuildNode& node, const BuildContext& ctx, std::array<BuildNode, 8>& children) {
		const std::vector<Tri>& tris = *ctx.tris;
		const float h = node.half * 0.5f;
		const float testHalf = h * ctx.boxExpand;

		for (uint32_t i = 0; i < 8; ++i) {
			BuildNode& child = children[i];
			child.center = node.center + V3{ (i & 1) ? h : -h, (i & 2) ? h : -h, (i & 4) ? h : -h };
			child.half = h;
			child.depth = node.depth + 1;
			child.tris.clear();
			child.tris.reserve(node.tris.size() / 2 + 1);
			for (uint32_t idx : node.tris) {
				if (TriangleBoxOverlap(child.center, testHalf, tris[idx])) {
					child.tris.push_back(idx);
				}
			}
			// 親中心 -> 子中心 の線分は子セル内に収まるので、子セルの三角形だけでワインディング数の変化が決まる
			child.centerWinding = node.centerWinding + SegmentWinding(node.center, child.center, tris, child.tris.data(), child.tris.size());
		}
	}

	void Classify(const BuildNode& node, const BuildContext& ctx, LeafList& out) {
		if (node.tris.empty()) {
			// 三角形と重ならないセルは全体が内部か外部。外部は捨てる
			if (node.centerWinding != 0) {
				out.leaves.push_back({ node.center, node.half, false, node.centerWinding, 0, 0 });
			}
			return;
		}

		if (node.depth >= ctx.maxDepth || node.tris.size() <= ctx.leafTriangleCount) {
			Leaf leaf{ node.center, node.half, true, node.centerWinding, uint32_t(out.triIndices.size()), uint32_t(node.tris.size()) };
			out.triIndices.insert(out.triIndices.end(), node.tris.begin(), node.tris.end());
			out.leaves.push_back(leaf);
			return;
		}

		std::array<BuildNode, 8> children;
		Subdivide(node, ctx, children);
		for (const BuildNode& child : children) {
			Classify(child, ctx, out);
		}
	}

	/// 並列化のため、浅い深さまでは逐次で展開してタスクに分ける
	void ExpandTasks(BuildNode&& node, const BuildContext& ctx, uint32_t taskDepth, std::vector<BuildNode>& tasks) {
		const bool isLeaf = node.tris.empty() || node.depth >= ctx.maxDepth || node.tris.size() <= ctx.leafTriangleCount;
		if (isLeaf || node.depth >= taskDepth) {
			tasks.push_back(std::move(node));
			return;
		}
		std::array<BuildNode, 8> children;
		Subdivide(node, ctx, children);
		for (BuildNode& child : children) {
			ExpandTasks(std::move(child), ctx, taskDepth, tasks);
		}
	}

	// ---------------------------------------------------------------------------------
	// モデル -> 三角形
	// ---------------------------------------------------------------------------------
	void CollectTriangles(const ModelData* modelData, std::vector<Tri>& out, uint32_t& vertexCount) {
		vertexCount = 0;
		for (const Mesh& mesh : modelData->meshes) {
			vertexCount += uint32_t(mesh.position.size());
			const size_t posCount = mesh.position.size();
			for (size_t i = 0; i + 2 < mesh.indices.size(); i += 3) {
				const uint32_t i0 = mesh.indices[i];
				const uint32_t i1 = mesh.indices[i + 1];
				const uint32_t i2 = mesh.indices[i + 2];
				if (i0 >= posCount || i1 >= posCount || i2 >= posCount) {
					continue;
				}
				const Vector3& a = mesh.position[i0];
				const Vector3& b = mesh.position[i1];
				const Vector3& c = mesh.position[i2];
				Tri tri{ { a.x, a.y, a.z }, { b.x, b.y, b.z }, { c.x, c.y, c.z } };
				// 面積0の三角形は判定を不安定にするだけなので除外
				if (Dot(Cross(tri.v1 - tri.v0, tri.v2 - tri.v0), Cross(tri.v1 - tri.v0, tri.v2 - tri.v0)) <= 0.0f) {
					continue;
				}
				out.push_back(tri);
			}
		}
	}

	uint32_t ResolveThreadCount(uint32_t requested) {
		uint32_t count = requested != 0 ? requested : std::thread::hardware_concurrency();
		return std::max(1u, count);
	}

	// ---------------------------------------------------------------------------------
	// Alias テーブル
	// ---------------------------------------------------------------------------------

	/// Vose の方法で Walker の Alias テーブルを作る (weights に比例した確率で O(1) で選べる)
	void BuildAliasTable(const std::vector<double>& weights, std::vector<float>& probability, std::vector<uint32_t>& alias) {
		const size_t n = weights.size();
		probability.assign(n, 1.0f);
		alias.resize(n);
		for (size_t i = 0; i < n; ++i) {
			alias[i] = uint32_t(i);
		}

		double sum = 0.0;
		for (double w : weights) {
			sum += w;
		}
		if (n == 0 || !(sum > 0.0)) {
			return;
		}

		std::vector<double> scaled(n);
		std::vector<uint32_t> underFull;
		std::vector<uint32_t> overFull;
		for (size_t i = 0; i < n; ++i) {
			scaled[i] = weights[i] * double(n) / sum;
			(scaled[i] < 1.0 ? underFull : overFull).push_back(uint32_t(i));
		}

		while (!underFull.empty() && !overFull.empty()) {
			const uint32_t s = underFull.back();
			underFull.pop_back();
			const uint32_t l = overFull.back();
			probability[s] = float(scaled[s]);
			alias[s] = l;
			scaled[l] = (scaled[l] + scaled[s]) - 1.0;
			if (scaled[l] < 1.0) {
				overFull.pop_back();
				underFull.push_back(l);
			}
		}
		// 残ったものは誤差で 1 付近になっているだけなので、確率 1 (自分自身) のままにする
	}

	// ---------------------------------------------------------------------------------
	// 保存
	// ---------------------------------------------------------------------------------
	bool GetBufferPath(const ModelData* modelData, std::string& path) {
		if (!modelData || modelData->modelName.size() <= ignoreString.size()) {
			return false;
		}
		path = bufferDirectory + modelData->modelName.substr(ignoreString.length()) + ".octree";
		return true;
	}

	/// ファイルを読み込み、壊れていないか・現在のモデルと一致するかを確認する
	bool ReadBuffer(const ModelData* modelData, const std::string& path, Particle::InnerModel::Volume& out) {
		BinaryManager bin;
		if (!bin.Boot(path)) {
			return false;
		}
		std::string signature;
		uint32_t version = 0;
		uint32_t triangleCount = 0;
		uint32_t vertexCount = 0;
		if (!bin.Reverse(signature) || signature != kSignature) { return false; }
		if (!bin.Reverse(version) || version != kFormatVersion) { return false; }
		bin.Reverse(triangleCount);
		bin.Reverse(vertexCount);
		bin.Reverse(out.min);
		bin.Reverse(out.max);
		if (bin.IsFailed()) {
			return false;
		}

		// モデルが変わっていたら作り直しが必要
		std::vector<Tri> tris;
		uint32_t currentVertexCount = 0;
		CollectTriangles(modelData, tris, currentVertexCount);
		if (triangleCount != tris.size() || vertexCount != currentVertexCount) {
			return false;
		}

		if (!bin.Reverse(out.cells) || !bin.Reverse(out.triangles) ||
			!bin.Reverse(out.aliasProbability) || !bin.Reverse(out.aliasIndex) || bin.IsFailed()) {
			return false;
		}

		// 範囲外アクセスしないよう、中身の整合性を確認する
		const size_t cellCount = out.cells.size();
		if (cellCount == 0 || out.aliasProbability.size() != cellCount || out.aliasIndex.size() != cellCount ||
			out.triangles.size() % 3 != 0) {
			return false;
		}
		const uint64_t storedTriangles = out.triangles.size() / 3;
		for (const Particle::InnerModel::Cell& cell : out.cells) {
			if (cell.IsMixed() && uint64_t(cell.triOffset) + cell.triCount > storedTriangles) {
				return false;
			}
		}
		for (uint32_t index : out.aliasIndex) {
			if (index >= cellCount) {
				return false;
			}
		}
		return true;
	}
}

bool Particle::InnerModel::EmitInnerModel(Emitter* emitter, SHEngine::BufferContainer* container, int emitCount, const ModelData* modelData,
	float scale, const Vector4& color) {
	if (!emitter || !container || !modelData) {
		return false;
	}

	// 保存済みの八分木を読む。無い/古い場合は作り直して保存する
	Volume volume;
	if (!LoadInnerModelBuffer(modelData, volume)) {
		if (!CreateInnderModelBuffer(modelData) || !LoadInnerModelBuffer(modelData, volume)) {
			return false;
		}
	}

	const uint32_t particleCount = emitter->GetParticleCount();
	const uint32_t count = (emitCount <= 0) ? particleCount : std::min(uint32_t(emitCount), particleCount);
	if (count == 0) {
		return true;
	}

	// シェーダーの Config と同じレイアウト
	struct EmitConfig {
		uint32_t emitCount;
		uint32_t cellCount;
		uint32_t seed;
		uint32_t maxAttempts;
		Vector4 color;
		float scale;
		float padding[3];
	};
	static std::random_device seedDevice;

	EmitConfig config{};
	config.emitCount = count;
	config.cellCount = uint32_t(volume.cells.size());
	config.seed = seedDevice();
	config.maxAttempts = 64;
	config.color = color;
	config.scale = scale;

	// 混在セルが無いモデルでも空のバッファは作れないため、最低1要素確保する
	if (volume.triangles.empty()) {
		volume.triangles.push_back({ 0.0f, 0.0f, 0.0f });
	}

	auto* configBuffer = container->Create(BufferType::CBV, sizeof(EmitConfig), 1, BufferNum::Single, "EmitConfig");
	auto* cellBuffer = container->Create(BufferType::SRV, sizeof(Cell), uint32_t(volume.cells.size()), BufferNum::Single, "Cell");
	auto* triangleBuffer = container->Create(BufferType::SRV, sizeof(Vector3), uint32_t(volume.triangles.size()), BufferNum::Single, "Triangle");
	auto* probabilityBuffer = container->Create(BufferType::SRV, sizeof(float), uint32_t(volume.aliasProbability.size()), BufferNum::Single, "Probability");
	auto* aliasBuffer = container->Create(BufferType::SRV, sizeof(uint32_t), uint32_t(volume.aliasIndex.size()), BufferNum::Single, "Alias");

	configBuffer->CopyBuffer(&config, sizeof(config));
	cellBuffer->CopyBuffer(volume.cells.data(), sizeof(Cell) * volume.cells.size());
	triangleBuffer->CopyBuffer(volume.triangles.data(), sizeof(Vector3) * volume.triangles.size());
	probabilityBuffer->CopyBuffer(volume.aliasProbability.data(), sizeof(float) * volume.aliasProbability.size());
	aliasBuffer->CopyBuffer(volume.aliasIndex.data(), sizeof(uint32_t) * volume.aliasIndex.size());

	// b0: パーティクル数, t0: indexList, u0~u3: position/rotation/scale/color は AddTriggerProcess が登録済み
	SHEngine::ComputeObject* process = emitter->AddTriggerProcess("EmitFunc/InnerModel");
	process->SetGPUBuffers(BufferType::CBV, 1, { configBuffer });
	process->SetGPUBuffers(BufferType::SRV, 1, { cellBuffer, triangleBuffer, probabilityBuffer, aliasBuffer });
	process->SetExecuteNum(int(count), 128);
	return true;
}

bool Particle::InnerModel::BuildInnerModelVolume(const ModelData* modelData, const BuildDesc& desc, Volume& out, DebugInfo* stats) {
	const auto startTime = std::chrono::steady_clock::now();
	out = {};

	if (!modelData) {
		return false;
	}

	std::vector<Tri> tris;
	uint32_t vertexCount = 0;
	CollectTriangles(modelData, tris, vertexCount);
	if (tris.empty()) {
		return false;
	}

	// ---- AABB ----
	V3 mn{ FLT_MAX, FLT_MAX, FLT_MAX };
	V3 mx{ -FLT_MAX, -FLT_MAX, -FLT_MAX };
	double totalArea = 0.0;
	for (const Tri& tri : tris) {
		for (const V3& v : { tri.v0, tri.v1, tri.v2 }) {
			mn = { std::min(mn.x, v.x), std::min(mn.y, v.y), std::min(mn.z, v.z) };
			mx = { std::max(mx.x, v.x), std::max(mx.y, v.y), std::max(mx.z, v.z) };
		}
		totalArea += 0.5 * Length(Cross(tri.v1 - tri.v0, tri.v2 - tri.v0));
	}
	out.min = ToVector3(mn);
	out.max = ToVector3(mx);

	const V3 size = mx - mn;
	const float longest = std::max({ size.x, size.y, size.z });
	if (!(longest > 0.0f)) {
		return false;
	}

	// ---- 最大深さ: ポリゴンの細かさとAABBの比で決め、kMaxDepth で打ち止める ----
	const uint32_t minDepth = std::min(desc.minDepth, kMaxDepth);
	const float avgEdge = float(std::sqrt(totalArea / double(tris.size())));
	const float targetCell = avgEdge * std::max(desc.cellScale, 1e-4f);
	uint32_t maxDepth = kMaxDepth;
	if (targetCell > 0.0f) {
		const int depth = int(std::ceil(std::log2(longest / targetCell)));
		maxDepth = uint32_t(std::clamp(depth, int(minDepth), int(kMaxDepth)));
	}

	// ---- ルート立方体 ----
	// 表面がセル境界やセル中心にぴったり乗ると判定が揺れるため、少し広げた上で中心を半端な量ずらす
	const float pad = longest * 1e-3f;
	const float rootHalf = longest * 0.5f + pad * 2.0f;
	const V3 rootCenter = (mn + mx) * 0.5f + V3{ pad * 0.3713f, pad * 0.6197f, pad * 0.1371f };

	BuildContext ctx{};
	ctx.tris = &tris;
	ctx.maxDepth = maxDepth;
	ctx.leafTriangleCount = desc.leafTriangleCount;
	ctx.boxExpand = 1.0f + 1e-4f;

	BuildNode root{};
	root.center = rootCenter;
	root.half = rootHalf;
	root.depth = 0;
	root.centerWinding = WindingByRay(rootCenter, tris);	// 全ポリゴンへの半直線判定はここだけ
	root.tris.resize(tris.size());
	for (uint32_t i = 0; i < uint32_t(tris.size()); ++i) {
		root.tris[i] = i;
	}

	// ---- 八分木の構築 (深さ2 = 最大64タスクに分けて並列化) ----
	const uint32_t threadCount = ResolveThreadCount(desc.threadCount);
	std::vector<BuildNode> tasks;
	ExpandTasks(std::move(root), ctx, 2, tasks);

	std::vector<LeafList> localLists(threadCount);
	{
		std::atomic<size_t> nextTask{ 0 };
		std::vector<std::thread> workers;
		for (uint32_t t = 0; t < threadCount; ++t) {
			workers.emplace_back([&, t]() {
				for (size_t i = nextTask++; i < tasks.size(); i = nextTask++) {
					Classify(tasks[i], ctx, localLists[t]);
				}
			});
		}
		for (auto& w : workers) {
			w.join();
		}
	}
	tasks.clear();
	tasks.shrink_to_fit();

	// ---- 葉 -> 保存形式 ----
	// 混在セルの三角形は、実行時の参照が連続するようセルごとに頂点を直接並べる (共有三角形は重複するが、1セル leafTriangleCount 程度)
	uint32_t insideCount = 0;
	uint32_t mixedCount = 0;
	std::vector<double> volumes;
	for (LeafList& list : localLists) {
		for (const Leaf& leaf : list.leaves) {
			Cell cell{};
			cell.center = ToVector3(leaf.center);
			cell.half = leaf.half;
			cell.centerWinding = leaf.centerWinding;
			if (leaf.mixed) {
				cell.flags = Cell::kCellFlagMixed;
				cell.triOffset = uint32_t(out.triangles.size() / 3);
				cell.triCount = leaf.triCount;
				for (uint32_t i = 0; i < leaf.triCount; ++i) {
					const Tri& tri = tris[list.triIndices[leaf.triOffset + i]];
					out.triangles.push_back(ToVector3(tri.v0));
					out.triangles.push_back(ToVector3(tri.v1));
					out.triangles.push_back(ToVector3(tri.v2));
				}
				++mixedCount;
			} else {
				++insideCount;
			}
			out.cells.push_back(cell);

			const double edge = double(leaf.half) * 2.0;
			volumes.push_back(edge * edge * edge);
		}
		list = {};
	}

	if (out.cells.empty()) {
		out = {};
		return false;
	}

	// ---- 体積比でセルを選ぶための Alias テーブル ----
	BuildAliasTable(volumes, out.aliasProbability, out.aliasIndex);

	if (stats) {
		stats->triangleCount = uint32_t(tris.size());
		stats->maxDepth = maxDepth;
		stats->insideLeafCount = insideCount;
		stats->mixedLeafCount = mixedCount;
		stats->buildMilliseconds = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - startTime).count();
	}
	return true;
}

bool Particle::InnerModel::CreateInnderModelBuffer(const ModelData* modelData, const BuildDesc& desc, DebugInfo* stats) {
	std::string path;
	if (!GetBufferPath(modelData, path)) {
		return false;
	}

	Volume volume;
	if (!BuildInnerModelVolume(modelData, desc, volume, stats)) {
		return false;
	}

	std::vector<Tri> tris;
	uint32_t vertexCount = 0;
	CollectTriangles(modelData, tris, vertexCount);

	try {
		BinaryManager bin;
		bin.Register(kSignature);
		bin.Register(kFormatVersion);
		bin.Register(uint32_t(tris.size()));
		bin.Register(vertexCount);
		bin.Register(volume.min);
		bin.Register(volume.max);
		bin.Register(volume.cells);
		bin.Register(volume.triangles);
		bin.Register(volume.aliasProbability);
		bin.Register(volume.aliasIndex);
		bin.Write(path);
	} catch (const std::exception&) {
		return false;
	}
	return true;
}

bool Particle::InnerModel::LoadInnerModelBuffer(const ModelData* modelData, Volume& out) {
	std::string path;
	if (!GetBufferPath(modelData, path)) {
		return false;
	}
	if (!ReadBuffer(modelData, path, out)) {
		out = {};
		return false;
	}
	return true;
}

bool Particle::InnerModel::ExistsInnerModelBuffer(const ModelData* modelData) {
	std::string path;
	if (!GetBufferPath(modelData, path)) {
		return false;
	}
	Volume volume;
	return ReadBuffer(modelData, path, volume);
}

bool Particle::InnerModel::SampleInnerModel(const Volume& volume, std::mt19937_64& rng, Vector3& out, uint32_t maxAttempts) {
	const uint32_t cellCount = uint32_t(volume.cells.size());
	if (cellCount == 0 || volume.aliasProbability.size() != cellCount || volume.aliasIndex.size() != cellCount) {
		return false;
	}

	std::uniform_int_distribution<uint32_t> pickCell(0, cellCount - 1);
	std::uniform_real_distribution<float> unit01(0.0f, 1.0f);
	std::uniform_real_distribution<float> unit(-1.0f, 1.0f);

	for (uint32_t attempt = 0; attempt < maxAttempts; ++attempt) {
		// Alias 法: 一様に選んだ枠を、確率 aliasProbability で自分 / それ以外で相方にする
		uint32_t index = pickCell(rng);
		if (unit01(rng) >= volume.aliasProbability[index]) {
			index = volume.aliasIndex[index];
		}

		const Cell& cell = volume.cells[index];
		const V3 center = ToV3(cell.center);
		const V3 p = center + V3{ unit(rng), unit(rng), unit(rng) } * cell.half;

		if (cell.IsMixed()) {
			const int winding = cell.centerWinding + SegmentWinding(center, p, volume.triangles.data() + size_t(cell.triOffset) * 3, cell.triCount);
			if (winding == 0) {
				// 外部だったのでセルの選択からやり直す
				continue;
			}
		}
		out = ToVector3(p);
		return true;
	}
	return false;
}

// 末尾でテンプレートがインスタンス化されるため、ファイル末尾で元に戻す
#pragma warning(pop)
