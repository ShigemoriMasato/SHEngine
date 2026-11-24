#include "InnerModel.h"

// BinaryManager の std::vector 登録処理 (RegisterValue<std::vector<T>>) に size_t -> uint32_t の暗黙変換警告(C4267)があり、
// 警告をエラーとして扱う設定ではインスタンス化した時点でビルドが通らないため、ここでのみ抑制する。
// (点群は最大でも数億点 = uint32_t に収まるため、変換による値の欠落は起きない)
#pragma warning(push)
#pragma warning(disable : 4267)
#include <Tool/Binary/BinaryManager.h>

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cfloat>
#include <filesystem>
#include <random>
#include <thread>

/*
 * モデル内部の候補点の事前計算
 *
 * 1. モデルAABBの最長辺を1辺とする立方体をルートにした八分木を作る
 * 2. 各セルについて「重なる三角形」を親セルのリストから絞り込む (Triangle-AABB の分離軸判定)
 *    - 重なる三角形が無い     : セル全体が内部 or 外部。中心点の内外がそのままセルの状態
 *    - 重なる三角形がある     : 内外混在。最大深さ/三角形数の条件に達するまで8分割
 * 3. セル中心の内外は、親の中心 -> 子の中心 の線分と「子セルの三角形」との交差回数の偶奇で引き継ぐ
 *    (線分は子セル内に収まるため、全ポリゴンを調べる必要が無い。全ポリゴンへの半直線判定はルートの1回のみ)
 * 4. 内部セル/混在セルを体積比で選んで点を打つ。混在セルでは セル中心 -> 点 の線分で同様に内外判定し、外部なら引き直す
 */

namespace {
	const std::string ignoreString = "Assets/";
	// BinaryManager の basePath (Assets/Binary/) からの相対パス
	const std::string bufferDirectory = "Particle/InnerModel/";
	const std::string binaryBasePath = "Assets/Binary/";

	const std::string kSignature = "InnerModelPoints";
	constexpr uint32_t kFormatVersion = 1;

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

	struct Tri {
		V3 v0, v1, v2;
	};

	// ---------------------------------------------------------------------------------
	// 交差判定
	// ---------------------------------------------------------------------------------

	/// Möller–Trumbore。o + t*d (tMin < t < tMax) と三角形の交差
	inline bool IntersectTriangle(const V3& o, const V3& d, const Tri& tri, float tMax) {
		const V3 e1 = tri.v1 - tri.v0;
		const V3 e2 = tri.v2 - tri.v0;
		const V3 p = Cross(d, e2);
		const float det = Dot(e1, p);
		if (std::fabs(det) < 1e-20f) {
			return false;
		}
		const float inv = 1.0f / det;
		const V3 s = o - tri.v0;
		const float u = Dot(s, p) * inv;
		if (u < 0.0f || u > 1.0f) {
			return false;
		}
		const V3 q = Cross(s, e1);
		const float v = Dot(d, q) * inv;
		if (v < 0.0f || u + v > 1.0f) {
			return false;
		}
		const float t = Dot(e2, q) * inv;
		return t > 0.0f && t <= tMax;
	}

	/// 線分 a -> b が tris[indices] と交差する回数の偶奇 (奇数なら true)
	inline bool SegmentParity(const V3& a, const V3& b, const std::vector<Tri>& tris, const uint32_t* indices, size_t count) {
		const V3 d = b - a;
		bool odd = false;
		for (size_t i = 0; i < count; ++i) {
			if (IntersectTriangle(a, d, tris[indices[i]], 1.0f)) {
				odd = !odd;
			}
		}
		return odd;
	}

	/// 全三角形に対する半直線の偶奇判定。辺・頂点に当たった場合の誤判定を避けるため3方向の多数決
	bool IsInsideByRay(const V3& p, const std::vector<Tri>& tris) {
		const V3 dirs[3] = {
			{ 1.0f, 0.00131f, 0.00077f },
			{ -0.00113f, 1.0f, 0.00191f },
			{ 0.00173f, -0.00097f, 1.0f },
		};
		int votes = 0;
		for (const V3& d : dirs) {
			bool odd = false;
			for (const Tri& tri : tris) {
				if (IntersectTriangle(p, d, tri, FLT_MAX)) {
					odd = !odd;
				}
			}
			votes += odd ? 1 : 0;
		}
		return votes >= 2;
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
		bool centerInside;
		std::vector<uint32_t> tris;	// このセルと重なる三角形
	};

	struct Leaf {
		V3 center;
		float half;
		bool mixed;			// false なら完全に内部
		bool centerInside;	// mixed の場合の中心点の内外
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
			// 親中心 -> 子中心 の線分は子セル内に収まるので、子セルの三角形だけで偶奇が決まる
			const bool odd = SegmentParity(node.center, child.center, tris, child.tris.data(), child.tris.size());
			child.centerInside = node.centerInside != odd;
		}
	}

	void Classify(const BuildNode& node, const BuildContext& ctx, LeafList& out) {
		if (node.tris.empty()) {
			// 三角形と重ならないセルは全体が内部か外部。外部は捨てる
			if (node.centerInside) {
				out.leaves.push_back({ node.center, node.half, false, true, 0, 0 });
			}
			return;
		}

		if (node.depth >= ctx.maxDepth || node.tris.size() <= ctx.leafTriangleCount) {
			Leaf leaf{ node.center, node.half, true, node.centerInside, uint32_t(out.triIndices.size()), uint32_t(node.tris.size()) };
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
	// 保存
	// ---------------------------------------------------------------------------------
	bool GetBufferPaths(const ModelData* modelData, std::string& metaPath, std::string& pointsPath) {
		if (!modelData || modelData->modelName.size() <= ignoreString.size()) {
			return false;
		}
		const std::string modelPath = modelData->modelName.substr(ignoreString.length());
		metaPath = bufferDirectory + modelPath + ".meta";
		pointsPath = bufferDirectory + modelPath + ".points";
		return true;
	}

	/// points ファイルの期待サイズ: [version 1][Vector id 1][allSize 4][count 4][Vector3 id 1][typeSize 4][data 12n]
	uint64_t ExpectedPointsFileSize(uint32_t pointCount) {
		return 1ull + 1ull + 4ull + 4ull + 1ull + 4ull + uint64_t(sizeof(Vector3)) * pointCount;
	}

	struct MetaData {
		uint32_t triangleCount = 0;
		uint32_t vertexCount = 0;
		uint32_t pointCount = 0;
		Vector3 min;
		Vector3 max;
	};

	bool ReadMeta(const std::string& metaPath, MetaData& meta) {
		BinaryManager bin;
		if (!bin.Boot(metaPath)) {
			return false;
		}
		std::string signature;
		uint32_t version = 0;
		if (!bin.Reverse(signature) || signature != kSignature) { return false; }
		if (!bin.Reverse(version) || version != kFormatVersion) { return false; }
		bin.Reverse(meta.triangleCount);
		bin.Reverse(meta.vertexCount);
		bin.Reverse(meta.pointCount);
		bin.Reverse(meta.min);
		bin.Reverse(meta.max);
		return !bin.IsFailed();
	}

	/// meta が現在のモデルと一致し、points ファイルのサイズが正しいか
	bool ValidateBuffer(const ModelData* modelData, const std::string& metaPath, const std::string& pointsPath, MetaData& meta) {
		if (!ReadMeta(metaPath, meta)) {
			return false;
		}

		std::vector<Tri> tris;
		uint32_t vertexCount = 0;
		CollectTriangles(modelData, tris, vertexCount);
		if (meta.triangleCount != tris.size() || meta.vertexCount != vertexCount) {
			return false;
		}

		std::error_code ec;
		const uint64_t fileSize = std::filesystem::file_size(binaryBasePath + pointsPath, ec);
		return !ec && fileSize == ExpectedPointsFileSize(meta.pointCount);
	}
}

void Particle::EmitInnnerModel(Emitter* emitter, const Matrix4x4& transform, int emitCount, const ModelData* modelData) {
	(void)emitter;
	(void)transform;
	(void)emitCount;

	if (!ExistsInnerModelBuffer(modelData)) {
		CreateInnderModelBuffer(modelData);
	}
}

bool Particle::BuildInnerModelPoints(const ModelData* modelData, const InnerModelBuildDesc& desc, InnerModelPoints& out, InnerModelBuildStats* stats) {
	const auto startTime = std::chrono::steady_clock::now();
	out.points.clear();

	if (!modelData || desc.pointCount == 0) {
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
	out.min = { mn.x, mn.y, mn.z };
	out.max = { mx.x, mx.y, mx.z };

	const V3 size = mx - mn;
	const float longest = std::max({ size.x, size.y, size.z });
	if (!(longest > 0.0f)) {
		return false;
	}

	// ---- 最大深さ: ポリゴンの細かさとAABBの比で決める ----
	const float avgEdge = float(std::sqrt(totalArea / double(tris.size())));
	const float targetCell = avgEdge * std::max(desc.cellScale, 1e-4f);
	uint32_t maxDepth = desc.maxDepthLimit;
	if (targetCell > 0.0f) {
		const int depth = int(std::ceil(std::log2(longest / targetCell)));
		maxDepth = uint32_t(std::clamp(depth, int(desc.minDepth), int(desc.maxDepthLimit)));
	}
	maxDepth = std::min(maxDepth, desc.maxDepthLimit);

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
	root.centerInside = IsInsideByRay(rootCenter, tris);	// 全ポリゴンへの半直線判定はここだけ
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

	LeafList all;
	for (LeafList& list : localLists) {
		const uint32_t offset = uint32_t(all.triIndices.size());
		for (Leaf& leaf : list.leaves) {
			leaf.triOffset += offset;
			all.leaves.push_back(leaf);
		}
		all.triIndices.insert(all.triIndices.end(), list.triIndices.begin(), list.triIndices.end());
		list = {};
	}

	if (all.leaves.empty()) {
		return false;
	}

	// ---- 体積の累積分布 ----
	std::vector<double> cdf(all.leaves.size());
	double accum = 0.0;
	uint32_t insideCount = 0;
	uint32_t mixedCount = 0;
	for (size_t i = 0; i < all.leaves.size(); ++i) {
		const double edge = double(all.leaves[i].half) * 2.0;
		accum += edge * edge * edge;
		cdf[i] = accum;
		all.leaves[i].mixed ? ++mixedCount : ++insideCount;
	}

	// ---- 点の生成 (スレッドごとに範囲を分担) ----
	out.points.resize(desc.pointCount);
	std::atomic<bool> failed{ false };
	std::atomic<uint64_t> rejected{ 0 };
	{
		std::vector<std::thread> workers;
		const uint32_t total = desc.pointCount;
		for (uint32_t t = 0; t < threadCount; ++t) {
			const uint32_t begin = uint32_t(uint64_t(total) * t / threadCount);
			const uint32_t end = uint32_t(uint64_t(total) * (t + 1) / threadCount);
			workers.emplace_back([&, begin, end, t]() {
				std::mt19937_64 rng(desc.seed ^ (0x9E3779B97F4A7C15ull * (t + 1)));
				std::uniform_real_distribution<double> pickDist(0.0, accum);
				std::uniform_real_distribution<float> unit(-1.0f, 1.0f);

				uint64_t localRejected = 0;
				// 混在セルばかりで内部がほぼ無い (メッシュが閉じていない等) 場合の無限ループ防止
				const uint64_t maxRejected = uint64_t(end - begin) * 64 + 1024;

				for (uint32_t i = begin; i < end; ++i) {
					while (true) {
						if (failed.load(std::memory_order_relaxed)) {
							return;
						}
						const double r = pickDist(rng);
						const size_t leafIndex = std::min(size_t(std::upper_bound(cdf.begin(), cdf.end(), r) - cdf.begin()), cdf.size() - 1);
						const Leaf& leaf = all.leaves[leafIndex];
						const V3 p = leaf.center + V3{ unit(rng), unit(rng), unit(rng) } * leaf.half;

						if (leaf.mixed) {
							const bool odd = SegmentParity(leaf.center, p, tris, all.triIndices.data() + leaf.triOffset, leaf.triCount);
							if (leaf.centerInside == odd) {
								// 外部だったので引き直す
								if (++localRejected > maxRejected) {
									failed = true;
									return;
								}
								continue;
							}
						}
						out.points[i] = { p.x, p.y, p.z };
						break;
					}
				}
				rejected += localRejected;
			});
		}
		for (auto& w : workers) {
			w.join();
		}
	}

	if (failed) {
		out.points.clear();
		return false;
	}

	if (stats) {
		stats->triangleCount = uint32_t(tris.size());
		stats->maxDepth = maxDepth;
		stats->insideLeafCount = insideCount;
		stats->mixedLeafCount = mixedCount;
		stats->rejectedCount = rejected;
		stats->buildMilliseconds = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - startTime).count();
	}
	return true;
}

bool Particle::CreateInnderModelBuffer(const ModelData* modelData, const InnerModelBuildDesc& desc, InnerModelBuildStats* stats) {
	std::string metaPath;
	std::string pointsPath;
	if (!GetBufferPaths(modelData, metaPath, pointsPath)) {
		return false;
	}

	InnerModelPoints result;
	if (!BuildInnerModelPoints(modelData, desc, result, stats)) {
		return false;
	}

	std::vector<Tri> tris;
	uint32_t vertexCount = 0;
	CollectTriangles(modelData, tris, vertexCount);

	try {
		// 点群本体 (大きいので meta と分け、存在確認時に読み込まずに済むようにする)
		{
			BinaryManager bin;
			bin.Register(result.points);
			bin.Write(pointsPath);
		}
		// meta は点群の書き込みに成功してから書く (meta があれば点群も揃っている状態にする)
		{
			BinaryManager bin;
			bin.Register(kSignature);
			bin.Register(kFormatVersion);
			bin.Register(uint32_t(tris.size()));
			bin.Register(vertexCount);
			bin.Register(uint32_t(result.points.size()));
			bin.Register(result.min);
			bin.Register(result.max);
			bin.Write(metaPath);
		}
	} catch (const std::exception&) {
		return false;
	}
	return true;
}

bool Particle::LoadInnerModelBuffer(const ModelData* modelData, InnerModelPoints& out) {
	std::string metaPath;
	std::string pointsPath;
	if (!GetBufferPaths(modelData, metaPath, pointsPath)) {
		return false;
	}

	MetaData meta;
	if (!ValidateBuffer(modelData, metaPath, pointsPath, meta)) {
		return false;
	}

	BinaryManager bin;
	if (!bin.Boot(pointsPath)) {
		return false;
	}
	if (!bin.Reverse(out.points) || out.points.size() != meta.pointCount) {
		out.points.clear();
		return false;
	}
	out.min = meta.min;
	out.max = meta.max;
	return true;
}

bool Particle::ExistsInnerModelBuffer(const ModelData* modelData) {
	std::string metaPath;
	std::string pointsPath;
	if (!GetBufferPaths(modelData, metaPath, pointsPath)) {
		return false;
	}
	MetaData meta;
	return ValidateBuffer(modelData, metaPath, pointsPath, meta);
}

// 末尾でテンプレートがインスタンス化されるため、ファイル末尾で元に戻す
#pragma warning(pop)
