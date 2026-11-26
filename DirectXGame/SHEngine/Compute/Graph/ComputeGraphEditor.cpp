#include "ComputeGraph.h"
#include "ComputeTrigger.h"

// ノード編集画面。USE_IMGUIが無効(Release)の場合は何もしない
#ifdef USE_IMGUI
#include <imgui/imgui.h>
#include <Compute/PSO/CSPSOManager.h>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <format>
#include <set>

using namespace SHEngine;

namespace {

	// ============================================================
	// 定数・小物
	// ============================================================

	// キャンバス座標での大きさ
	constexpr float kHeaderHeight = 26.0f;
	constexpr float kRowHeight = 22.0f;
	constexpr float kPadding = 8.0f;
	constexpr float kPinRadius = 5.0f;
	constexpr float kMinNodeWidth = 170.0f;
	constexpr float kMinZoom = 0.3f;
	constexpr float kMaxZoom = 2.5f;

	ImVec2 Add(const ImVec2& a, const ImVec2& b) { return { a.x + b.x, a.y + b.y }; }
	ImVec2 Sub(const ImVec2& a, const ImVec2& b) { return { a.x - b.x, a.y - b.y }; }
	ImVec2 Mul(const ImVec2& a, float s) { return { a.x * s, a.y * s }; }
	float LengthSq(const ImVec2& a) { return a.x * a.x + a.y * a.y; }

	ImU32 PinColor(SlotRequest::Type type) {
		switch (type) {
		case SlotRequest::Type::CBV: return IM_COL32(230, 160, 60, 255);
		case SlotRequest::Type::SRV: return IM_COL32(90, 200, 110, 255);
		case SlotRequest::Type::UAV: return IM_COL32(220, 90, 90, 255);
		case SlotRequest::Type::Texture: return IM_COL32(90, 150, 230, 255);
		}
		return IM_COL32(200, 200, 200, 255);
	}
	constexpr ImU32 kExecColor = IM_COL32(235, 235, 235, 255);
	constexpr ImU32 kOutputColor = IM_COL32(200, 200, 200, 255);
	constexpr ImU32 kErrorColor = IM_COL32(240, 80, 80, 255);
	constexpr ImU32 kUnknownColor = IM_COL32(230, 210, 80, 255);

	const char* PinPrefix(SlotRequest::Type type) {
		switch (type) {
		case SlotRequest::Type::CBV: return "b";
		case SlotRequest::Type::SRV: return "t";
		case SlotRequest::Type::UAV: return "u";
		case SlotRequest::Type::Texture: return "t";
		}
		return "?";
	}

	// "Particle/Update.CS.hlsl" → "Update"
	std::string ShaderStem(const std::string& shaderName) {
		std::string stem = shaderName;
		size_t slash = stem.find_last_of('/');
		if (slash != std::string::npos) stem = stem.substr(slash + 1);
		const std::string ext = ".CS.hlsl";
		if (stem.size() > ext.size() && stem.ends_with(ext)) stem.resize(stem.size() - ext.size());
		return stem;
	}

	std::string BufferTypeText(const GPUBuffer* buffer) {
		if (!buffer) return "-";
		if (buffer->IsTexture()) return "Texture";
		std::string text;
		const uint8_t type = buffer->GetBufferType();
		auto append = [&](BufferType t, const char* name) {
			if (type & t) {
				if (!text.empty()) text += "|";
				text += name;
			}
			};
		append(BufferType::CBV, "CBV");
		append(BufferType::SRV, "SRV");
		append(BufferType::UAV, "UAV");
		return std::format("{} {}x{}", text, buffer->GetStrideInBytes(), buffer->GetNum());
	}

	// ============================================================
	// 状態
	// ============================================================

	enum class PinRole : uint8_t { ExecIn, ExecOut, Input, Output };

	struct PinHandle {
		GraphNodeID node = kInvalidNodeID;
		PinRole role = PinRole::ExecIn;
		GraphPinKey key;	// Inputのみ
		bool IsOutput() const { return role == PinRole::ExecOut || role == PinRole::Output; }
	};

	struct PinLayout {
		PinHandle handle;
		ImVec2 screen;
		ImU32 color = kOutputColor;
		bool missing = false;	// シェーダーから消えたレジスタにつながっているリンク
	};

	struct RowLayout {
		std::string text;
		ImU32 color = IM_COL32(230, 230, 230, 255);
	};

	struct NodeLayout {
		GraphNodeID id = kInvalidNodeID;
		ImVec2 min, max;			// スクリーン座標
		std::string title;
		ImU32 headerColor = IM_COL32(70, 70, 80, 255);
		std::vector<RowLayout> rows;
		std::vector<PinLayout> pins;
	};

	enum class DragKind : uint8_t { None, Node, Link };

	struct ShaderTree {
		std::map<std::string, ShaderTree> folders;
		std::vector<std::pair<std::string, std::string>> files;	// 表示名, パス
	};

	struct EditorState {
		std::string currentGraph;
		GraphNodeID selectedNode = kInvalidNodeID;
		int selectedLink = -1;

		DragKind drag = DragKind::None;
		GraphNodeID dragNode = kInvalidNodeID;
		ImVec2 dragOffset;
		bool dragMoved = false;
		PinHandle dragPin;
		bool rightDragged = false;

		// 右クリックメニューの対象
		enum class MenuTarget : uint8_t { Canvas, Node, Pin, ExecDrop };
		MenuTarget menuTarget = MenuTarget::Canvas;
		GraphNodeID menuNode = kInvalidNodeID;
		PinHandle menuPin;
		Vector2 menuPosition;	// キャンバス座標

		char argumentInput[128] = "";
		char textBuffer[256] = "";

		// シェーダー一覧
		bool shaderListLoaded = false;
		std::vector<std::string> shaderFiles;
		ShaderTree shaderTree;

		// 保存済みのグラフファイル
		bool fileListLoaded = false;
		std::vector<std::string> graphFiles;

		std::string message;
		double messageTime = -100.0;
	};

	EditorState& Editor() {
		static EditorState state;
		return state;
	}

	void ShowMessage(const std::string& message) {
		Editor().message = message;
		Editor().messageTime = ImGui::GetTime();
	}

	// ============================================================
	// ファイル一覧
	// ============================================================

	void LoadShaderList() {
		auto& editor = Editor();
		editor.shaderFiles.clear();
		editor.shaderTree = {};
		namespace fs = std::filesystem;
		const fs::path root = "Assets/Shader/Compute";
		std::error_code ec;
		if (fs::exists(root, ec)) {
			for (const auto& entry : fs::recursive_directory_iterator(root, ec)) {
				if (!entry.is_regular_file()) continue;
				std::string relative = fs::relative(entry.path(), root, ec).generic_string();
				if (relative.ends_with(".CS.hlsl")) {
					editor.shaderFiles.push_back(relative);
				}
			}
		}
		std::sort(editor.shaderFiles.begin(), editor.shaderFiles.end());
		for (const auto& path : editor.shaderFiles) {
			ShaderTree* current = &editor.shaderTree;
			size_t begin = 0;
			size_t slash = path.find('/');
			while (slash != std::string::npos) {
				current = &current->folders[path.substr(begin, slash - begin)];
				begin = slash + 1;
				slash = path.find('/', begin);
			}
			current->files.emplace_back(path.substr(begin), path);
		}
		editor.shaderListLoaded = true;
	}

	void LoadGraphFileList() {
		auto& editor = Editor();
		editor.graphFiles.clear();
		namespace fs = std::filesystem;
		const fs::path root = "Assets/Binary/ComputeGraph";
		std::error_code ec;
		if (fs::exists(root, ec)) {
			for (const auto& entry : fs::recursive_directory_iterator(root, ec)) {
				if (!entry.is_regular_file() || entry.path().extension() != ".bin") continue;
				std::string relative = fs::relative(entry.path(), root, ec).generic_string();
				relative.resize(relative.size() - 4);
				editor.graphFiles.push_back(relative);
			}
		}
		editor.fileListLoaded = true;
	}

	// @brief シェーダーを選ぶメニュー。選ばれたらパスを返す
	std::string DrawShaderMenu(const ShaderTree& tree) {
		std::string selected;
		for (const auto& [name, child] : tree.folders) {
			if (ImGui::BeginMenu(name.c_str())) {
				std::string result = DrawShaderMenu(child);
				if (!result.empty()) selected = result;
				ImGui::EndMenu();
			}
		}
		for (const auto& [label, path] : tree.files) {
			if (ImGui::MenuItem(label.c_str())) {
				selected = path;
			}
		}
		return selected;
	}

	// ============================================================
	// グラフの補助
	// ============================================================

	std::vector<ComputeTrigger*> LiveTriggers(const std::string& graphName) {
		std::vector<ComputeTrigger*> result;
		for (ComputeTrigger* trigger : ComputeGraphRegistry::GetTriggers()) {
			if (trigger->GetName() == graphName) result.push_back(trigger);
		}
		return result;
	}

	// 生存中のTriggerでBindされている引数のバッファ(最初に見つかったもの)
	GPUBuffer* FindBoundArgument(const std::string& graphName, const std::string& argumentName) {
		for (ComputeTrigger* trigger : LiveTriggers(graphName)) {
			auto it = trigger->GetArguments().buffers.find(argumentName);
			if (it != trigger->GetArguments().buffers.end()) return it->second;
		}
		return nullptr;
	}

	// 表示に使う引数(最初に見つかった生存中のTriggerのもの)。なければnullptr
	const ComputeArguments* PreviewArguments(const std::string& graphName) {
		for (ComputeTrigger* trigger : ComputeGraphRegistry::GetTriggers()) {
			if (trigger->GetName() == graphName) return &trigger->GetArguments();
		}
		return nullptr;
	}

	// 出力ノードが指すバッファ(エディタ表示用)
	GPUBuffer* PreviewSource(ComputeGraph& graph, GraphNode& node) {
		if (node.kind == GraphNodeKind::Argument) {
			return FindBoundArgument(graph.GetName(), node.argumentName);
		}
		static const ComputeArguments kEmpty;
		return graph.ResolveSource(node, kEmpty);
	}

	// 出力ノードをピンにつないでよいか。分からない(Bindされていない引数など)場合はtrue
	bool IsCompatible(ComputeGraph& graph, GraphNode& source, const SlotRequest& request, bool* known = nullptr) {
		GPUBuffer* buffer = PreviewSource(graph, source);
		if (known) *known = buffer != nullptr;
		return !buffer || ResourceRegistry::CanBindBuffer(buffer, request);
	}

	GraphNode& CreateResourceNode(ComputeGraph& graph, const ResourceRef& ref, const Vector2& position) {
		GraphNode& node = graph.AddNode(GraphNodeKind::Resource, position);
		node.resource = ref;
		node.resourceName = ResourceRegistry::GetPersistentName(ref);
		return node;
	}

	GraphNode& CreateDispatchNode(ComputeGraph& graph, const std::string& shaderName, const Vector2& position) {
		GraphNode& node = graph.AddNode(GraphNodeKind::Dispatch, position);
		node.shaderName = shaderName;
		graph.RefreshPins(node);
		// numthreadsのXに合わせておく(1グループ分)
		node.dispatchSize[0] = node.numThreads[0];
		// コードからSetCountされた数が1つだけなら、それをDispatch数にする(Emitterの "Particles" など)
		if (const ComputeArguments* arguments = PreviewArguments(graph.GetName()); arguments && arguments->counts.size() == 1) {
			node.dispatchMode = DispatchMode::ArgumentCount;
			node.countArgument = arguments->counts.begin()->first;
		}
		return node;
	}

	// @brief ノードのシェーダーをファイルから読み直してコンパイルし直す
	void RecompileShader(ComputeGraph& graph, GraphNode& node) {
		PSO::CSPSOManager* psoManager = ComputeGraphRegistry::GetPSOManager();
		if (!psoManager || node.shaderName.empty()) {
			return;
		}
		const bool succeeded = psoManager->RecompileShader(node.shaderName);
		// 同じシェーダーを使う他のグラフも、次の実行前にピンを作り直す
		for (auto& [name, other] : ComputeGraphRegistry::GetGraphs()) {
			other->RequestRefresh();
		}
		graph.RefreshPins(node);
		ShowMessage(succeeded
			? "再コンパイルしました: " + node.shaderName
			: "コンパイルに失敗しました (ログを確認してください): " + node.shaderName);
	}

	// 入力ピンの左に置く位置
	Vector2 LeftOf(const GraphNode& node) {
		return { node.position.x - 240.0f, node.position.y };
	}

	// @brief ピン同士をつなぐ
	bool TryConnect(ComputeGraph& graph, const PinHandle& a, const PinHandle& b) {
		if (a.node == b.node || a.IsOutput() == b.IsOutput()) {
			return false;
		}
		const PinHandle& out = a.IsOutput() ? a : b;
		const PinHandle& in = a.IsOutput() ? b : a;

		if (out.role == PinRole::ExecOut && in.role == PinRole::ExecIn) {
			if (!graph.SetNext(out.node, in.node)) {
				ShowMessage("実行順が循環するため、つなげません");
				return false;
			}
			return true;
		}
		if (out.role == PinRole::Output && in.role == PinRole::Input) {
			GraphNode* source = graph.GetNode(out.node);
			GraphNode* target = graph.GetNode(in.node);
			if (!source || !target) return false;
			const GraphPin* pin = ComputeGraph::FindPin(*target, in.key);
			if (pin && !IsCompatible(graph, *source, pin->request)) {
				ShowMessage("種類・サイズが合わないため割り当てられません: " + pin->request.label);
				return false;
			}
			graph.Connect(out.node, in.node, in.key);
			return true;
		}
		ShowMessage("実行順のピンとリソースのピンはつなげません");
		return false;
	}

	// @brief Resourceウィンドウで選ばせて、選ばれたリソースのノードを作ってピンにつなぐ
	void SelectResourceForPin(ComputeGraph& graph, GraphNodeID nodeID, const GraphPinKey& key, const Vector2& position) {
		GraphNode* node = graph.GetNode(nodeID);
		if (!node) return;
		const GraphPin* pin = ComputeGraph::FindPin(*node, key);
		if (!pin) return;
		std::string graphName = graph.GetName();
		GraphPinKey pinKey = pin->key;
		ResourceRegistry::BeginSelection(pin->request, [graphName, nodeID, pinKey, position](const ResourceRef& ref) {
			ComputeGraph* target = ComputeGraphRegistry::Find(graphName);
			if (!target || !target->GetNode(nodeID)) return;
			GraphNode& created = CreateResourceNode(*target, ref, position);
			target->Connect(created.id, nodeID, pinKey);
			});
		ShowMessage("Resourceウィンドウで割り当てるリソースを選んでください");
	}

	// ============================================================
	// レイアウト
	// ============================================================

	struct Canvas {
		ImVec2 origin;	// キャンバスのスクリーン上の左上
		ImVec2 size;
		Vector2 scroll;
		float zoom = 1.0f;

		ImVec2 ToScreen(const Vector2& p) const { return { origin.x + (p.x + scroll.x) * zoom, origin.y + (p.y + scroll.y) * zoom }; }
		Vector2 ToCanvas(const ImVec2& s) const { return { (s.x - origin.x) / zoom - scroll.x, (s.y - origin.y) / zoom - scroll.y }; }
	};

	float TextWidth(const std::string& text) {
		return ImGui::GetFont()->CalcTextSizeA(ImGui::GetStyle().FontSizeBase, FLT_MAX, 0.0f, text.c_str()).x;
	}

	NodeLayout BuildLayout(ComputeGraph& graph, GraphNode& node, const Canvas& canvas) {
		NodeLayout layout;
		layout.id = node.id;

		// 入力ピン(行の左), 出力ピン(行の右)の行番号を覚えておき、幅が決まってから位置を計算する
		struct PendingPin { PinLayout pin; int row; bool left; };
		std::vector<PendingPin> pending;
		auto addRow = [&](const std::string& text, ImU32 color = IM_COL32(230, 230, 230, 255)) {
			layout.rows.push_back({ text, color });
			return int(layout.rows.size()) - 1;
			};
		auto addPin = [&](PinRole role, int row, ImU32 color, const GraphPinKey& key = {}, bool missing = false) {
			PinLayout pin;
			pin.handle = { node.id, role, key };
			pin.color = color;
			pin.missing = missing;
			const bool left = role == PinRole::ExecIn || role == PinRole::Input;
			pending.push_back({ pin, row, left });
			};

		switch (node.kind) {
		case GraphNodeKind::Trigger:
		{
			layout.title = "Trigger";
			layout.headerColor = IM_COL32(150, 60, 60, 255);
			addRow(graph.GetName());
			addPin(PinRole::ExecOut, -1, kExecColor);
			break;
		}
		case GraphNodeKind::Dispatch:
		{
			layout.title = node.shaderName.empty() ? "(シェーダー未設定)" : ShaderStem(node.shaderName);
			layout.headerColor = node.enabled ? IM_COL32(50, 90, 150, 255) : IM_COL32(70, 70, 70, 255);
			addPin(PinRole::ExecIn, -1, kExecColor);
			addPin(PinRole::ExecOut, -1, kExecColor);
			const ComputeArguments* arguments = PreviewArguments(graph.GetName());
			for (const auto& pin : node.pins) {
				const GraphLink* link = graph.FindLink(node.id, pin.key);
				std::string text = std::format("{}{} {}", PinPrefix(pin.key.type), pin.key.registerIndex, pin.key.name);
				if (pin.key.type == SlotRequest::Type::Texture) text += pin.request.writable ? " [RWTex]" : " [Tex]";
				ImU32 color = link ? IM_COL32(230, 230, 230, 255) : IM_COL32(150, 150, 150, 255);
				// つながっていなくても既定のバッファがあれば使われる
				if (!link && arguments) {
					if (const DefaultBinding* binding = arguments->FindDefault(pin.key, pin.request.writable)) {
						const bool ok = ResourceRegistry::CanBindBuffer(binding->buffer, pin.request);
						text += "  = " + binding->label;
						color = ok ? IM_COL32(150, 190, 230, 255) : kErrorColor;
					}
				}
				int row = addRow(text, color);
				addPin(PinRole::Input, row, PinColor(pin.key.type), pin.key);
			}
			// シェーダーから消えたレジスタにつながっているリンク
			for (const auto& link : graph.GetLinks()) {
				if (link.to == node.id && !ComputeGraph::FindPin(node, link.pin)) {
					int row = addRow(std::format("{}{} {} (消失)", PinPrefix(link.pin.type), link.pin.registerIndex, link.pin.name), kErrorColor);
					addPin(PinRole::Input, row, kErrorColor, link.pin, true);
				}
			}
			if (node.useBindlessTexture) {
				addRow("Texture2D[] (バインドレス)", IM_COL32(150, 150, 150, 255));
			}
			const char* modeName[] = { "スレッド数", "グループ数", "要素数", "引数" };
			addRow(std::format("{} / numthreads {}x{}x{}", modeName[int(node.dispatchMode)],
				node.numThreads[0], node.numThreads[1], node.numThreads[2]), IM_COL32(170, 170, 200, 255));
			if (!node.shaderError.empty()) {
				addRow(node.shaderError, kErrorColor);
			} else if (!node.status.empty()) {
				addRow(node.status, kErrorColor);
			}
			break;
		}
		case GraphNodeKind::Resource:
		{
			layout.title = "Resource";
			layout.headerColor = IM_COL32(60, 110, 70, 255);
			GPUBuffer* buffer = PreviewSource(graph, node);
			int row = addRow(node.resourceName.empty() ? "(未設定)" : node.resourceName, buffer ? IM_COL32(230, 230, 230, 255) : kErrorColor);
			addPin(PinRole::Output, row, buffer && buffer->IsTexture() ? PinColor(SlotRequest::Type::Texture) : kOutputColor);
			addRow(buffer ? BufferTypeText(buffer) : "(見つかりません)", IM_COL32(150, 150, 150, 255));
			break;
		}
		case GraphNodeKind::Argument:
		{
			layout.title = "Argument";
			layout.headerColor = IM_COL32(120, 90, 40, 255);
			GPUBuffer* buffer = PreviewSource(graph, node);
			int row = addRow(node.argumentName.empty() ? "(名前未設定)" : node.argumentName);
			addPin(PinRole::Output, row, buffer ? kOutputColor : kUnknownColor);
			addRow(buffer ? BufferTypeText(buffer) : "(Bindされていません)", IM_COL32(150, 150, 150, 255));
			break;
		}
		}

		// 幅と高さ(キャンバス座標)
		float width = std::max(kMinNodeWidth, TextWidth(layout.title) + kPadding * 2.0f + kPinRadius * 6.0f);
		for (const auto& row : layout.rows) {
			width = std::max(width, TextWidth(row.text) + kPadding * 2.0f + kPinRadius * 4.0f);
		}
		const float height = kHeaderHeight + kRowHeight * float(layout.rows.size()) + kPadding * 0.5f;

		layout.min = canvas.ToScreen(node.position);
		layout.max = canvas.ToScreen({ node.position.x + width, node.position.y + height });

		for (auto& item : pending) {
			float y = item.row < 0
				? node.position.y + kHeaderHeight * 0.5f
				: node.position.y + kHeaderHeight + kRowHeight * (float(item.row) + 0.5f);
			float x = item.left ? node.position.x : node.position.x + width;
			item.pin.screen = canvas.ToScreen({ x, y });
			layout.pins.push_back(item.pin);
		}
		return layout;
	}

	// ============================================================
	// 描画
	// ============================================================

	// @brief ズームに合わせた文字サイズ。数段階に丸める
	// @note ImGui 1.92は文字サイズごとにフォントアトラスへ文字を描き込むため、ズームの値をそのまま使うと
	//       サイズの種類だけアトラスが膨らみ、拡張(テクスチャの作り直し)が頻繁に起きる
	float ZoomedFontSize(float zoom) {
		// 枠からはみ出さないよう、ズーム以下で最大の段階を選ぶ
		constexpr float kSteps[] = { 0.5f, 0.625f, 0.75f, 0.875f, 1.0f, 1.25f, 1.5f, 2.0f, 2.5f };
		float scale = kSteps[0];
		for (float step : kSteps) {
			if (step <= zoom + 0.001f) scale = step;
		}
		return std::round(ImGui::GetStyle().FontSizeBase * scale);
	}

	void DrawText(ImDrawList* drawList, const Canvas& canvas, const ImVec2& pos, ImU32 color, const std::string& text) {
		if (canvas.zoom < 0.4f) return;	// 縮小しすぎたら文字は省く
		drawList->AddText(ImGui::GetFont(), ZoomedFontSize(canvas.zoom), pos, color, text.c_str());
	}

	void DrawLinkCurve(ImDrawList* drawList, const ImVec2& from, const ImVec2& to, ImU32 color, float thickness) {
		float dx = std::max(std::abs(to.x - from.x) * 0.5f, 30.0f);
		drawList->AddBezierCubic(from, { from.x + dx, from.y }, { to.x - dx, to.y }, to, color, thickness);
	}

	// ベジェ曲線とマウスの距離の2乗(リンクのホバー判定用)
	float LinkDistanceSq(const ImVec2& from, const ImVec2& to, const ImVec2& point) {
		float dx = std::max(std::abs(to.x - from.x) * 0.5f, 30.0f);
		ImVec2 p1 = { from.x + dx, from.y };
		ImVec2 p2 = { to.x - dx, to.y };
		float best = FLT_MAX;
		ImVec2 prev = from;
		constexpr int kSegments = 24;
		for (int i = 1; i <= kSegments; ++i) {
			float t = float(i) / kSegments;
			float u = 1.0f - t;
			ImVec2 p = Add(Add(Mul(from, u * u * u), Mul(p1, 3 * u * u * t)), Add(Mul(p2, 3 * u * t * t), Mul(to, t * t * t)));
			// 線分との距離
			ImVec2 seg = Sub(p, prev);
			float lenSq = LengthSq(seg);
			float s = lenSq > 0.0f ? std::clamp(((point.x - prev.x) * seg.x + (point.y - prev.y) * seg.y) / lenSq, 0.0f, 1.0f) : 0.0f;
			ImVec2 closest = Add(prev, Mul(seg, s));
			best = std::min(best, LengthSq(Sub(point, closest)));
			prev = p;
		}
		return best;
	}

	void DrawNode(ImDrawList* drawList, const Canvas& canvas, const NodeLayout& layout, bool selected, const PinLayout* hoveredPin,
		const std::set<const PinLayout*>& highlighted, bool dragging) {
		const float rounding = 4.0f * canvas.zoom;
		const float headerBottom = layout.min.y + kHeaderHeight * canvas.zoom;

		drawList->AddRectFilled(layout.min, layout.max, IM_COL32(35, 35, 40, 235), rounding);
		drawList->AddRectFilled(layout.min, { layout.max.x, headerBottom }, layout.headerColor, rounding, ImDrawFlags_RoundCornersTop);
		drawList->AddRect(layout.min, layout.max, selected ? IM_COL32(255, 200, 80, 255) : IM_COL32(90, 90, 100, 255), rounding, 0, selected ? 2.0f : 1.0f);

		const float textOffsetY = (kRowHeight - ImGui::GetStyle().FontSizeBase) * 0.5f * canvas.zoom;
		DrawText(drawList, canvas, { layout.min.x + kPadding * 2.0f * canvas.zoom, layout.min.y + (kHeaderHeight - ImGui::GetStyle().FontSizeBase) * 0.5f * canvas.zoom },
			IM_COL32(255, 255, 255, 255), layout.title);
		for (size_t i = 0; i < layout.rows.size(); ++i) {
			ImVec2 pos = { layout.min.x + kPadding * 1.5f * canvas.zoom, headerBottom + kRowHeight * float(i) * canvas.zoom + textOffsetY };
			DrawText(drawList, canvas, pos, layout.rows[i].color, layout.rows[i].text);
		}

		for (const auto& pin : layout.pins) {
			const float radius = kPinRadius * canvas.zoom;
			bool dim = dragging && !highlighted.contains(&pin);
			ImU32 color = dim ? IM_COL32(80, 80, 80, 255) : pin.color;
			if (pin.handle.role == PinRole::ExecIn || pin.handle.role == PinRole::ExecOut) {
				// 実行順のピンは三角形
				ImVec2 c = pin.screen;
				drawList->AddTriangleFilled({ c.x - radius, c.y - radius }, { c.x - radius, c.y + radius }, { c.x + radius, c.y }, color);
			} else {
				drawList->AddCircleFilled(pin.screen, radius, color);
			}
			if (&pin == hoveredPin || (dragging && highlighted.contains(&pin))) {
				drawList->AddCircle(pin.screen, radius + 3.0f, IM_COL32(255, 255, 255, 200), 0, 1.5f);
			}
		}
	}

	// ============================================================
	// サイドパネル
	// ============================================================

	bool InputString(const char* label, std::string& value) {
		auto& buffer = Editor().textBuffer;
		std::snprintf(buffer, sizeof(buffer), "%s", value.c_str());
		if (ImGui::InputText(label, buffer, sizeof(buffer), ImGuiInputTextFlags_EnterReturnsTrue) || ImGui::IsItemDeactivatedAfterEdit()) {
			if (value != buffer) {
				value = buffer;
				return true;
			}
		}
		return false;
	}

	void DrawDispatchProperties(ComputeGraph& graph, GraphNode& node) {
		auto& editor = Editor();

		ImGui::TextDisabled("Shader");
		if (ImGui::Button((node.shaderName.empty() ? "(選択してください)" : node.shaderName).c_str(), ImVec2(-1.0f, 0.0f))) {
			if (!editor.shaderListLoaded) LoadShaderList();
			ImGui::OpenPopup("SelectShader");
		}
		if (ImGui::BeginPopup("SelectShader")) {
			std::string selected = DrawShaderMenu(editor.shaderTree);
			if (!selected.empty()) {
				node.shaderName = selected;
				graph.MarkEdited();
				graph.RefreshPins(node);
			}
			ImGui::EndPopup();
		}

		if (ImGui::Checkbox("有効", &node.enabled)) graph.MarkEdited();
		ImGui::Text("numthreads: %u x %u x %u", node.numThreads[0], node.numThreads[1], node.numThreads[2]);

		const char* modes[] = { "スレッド数を指定", "グループ数を指定", "リソースの要素数", "引数の数 (SetCount)" };
		int mode = int(node.dispatchMode);
		if (ImGui::Combo("Dispatch", &mode, modes, IM_ARRAYSIZE(modes))) {
			node.dispatchMode = DispatchMode(mode);
			graph.MarkEdited();
		}

		switch (node.dispatchMode) {
		case DispatchMode::Threads:
		case DispatchMode::Groups:
			if (ImGui::InputScalarN(node.dispatchMode == DispatchMode::Threads ? "Threads" : "Groups", ImGuiDataType_U32, node.dispatchSize, 3)) {
				graph.MarkEdited();
			}
			break;
		case DispatchMode::ResourceCount:
		{
			std::string current = std::format("{}{} {}", PinPrefix(node.countPin.type), node.countPin.registerIndex, node.countPin.name);
			if (ImGui::BeginCombo("ピン", node.countPin.name.empty() ? "(選択)" : current.c_str())) {
				for (const auto& pin : node.pins) {
					std::string label = std::format("{}{} {}", PinPrefix(pin.key.type), pin.key.registerIndex, pin.key.name);
					if (ImGui::Selectable(label.c_str(), pin.key.SameRegister(node.countPin))) {
						node.countPin = pin.key;
						graph.MarkEdited();
					}
				}
				ImGui::EndCombo();
			}
			break;
		}
		case DispatchMode::ArgumentCount:
		{
			if (InputString("数の名前", node.countArgument)) graph.MarkEdited();
			// コードでSetCountされている名前
			std::set<std::string> names;
			for (ComputeTrigger* trigger : LiveTriggers(graph.GetName())) {
				for (const auto& [name, count] : trigger->GetArguments().counts) names.insert(name);
			}
			for (const auto& name : names) {
				if (ImGui::SmallButton(name.c_str())) {
					node.countArgument = name;
					graph.MarkEdited();
				}
				ImGui::SameLine();
			}
			ImGui::NewLine();
			break;
		}
		}

		// サンプラー(シェーダーがSamplerStateかバインドレスのテクスチャを使う場合のみ)
		if (node.useSampler || node.useBindlessTexture) {
			if (ImGui::TreeNode("サンプラー")) {
				struct SamplerFlag { const char* label; PSO::SamplerID id; };
				static const SamplerFlag kFlags[] = {
					{ "Default (Repeat)", PSO::SamplerID::Default },
					{ "ClampT", PSO::SamplerID::ClampT }, { "MirrorT", PSO::SamplerID::MirrorT },
					{ "ClampS", PSO::SamplerID::ClampS }, { "MirrorS", PSO::SamplerID::MirrorS },
					{ "MagNearest", PSO::SamplerID::MagNearest }, { "MagLinear", PSO::SamplerID::MagLinear },
					{ "MinNearest", PSO::SamplerID::MinNearest }, { "MinLinear", PSO::SamplerID::MinLinear },
					{ "Point", PSO::SamplerID::Point },
					{ "MinLinearMipmapNearest", PSO::SamplerID::MinLinearMipmapNearest },
					{ "MinNearestMipmapLinear", PSO::SamplerID::MinNearestMipmapLinear },
					{ "MinLinearMipmapLinear", PSO::SamplerID::MinLinearMipmapLinear },
					{ "ClampClamp_MinMagNearest", PSO::SamplerID::ClampClamp_MinMagNearest },
				};
				for (const auto& flag : kFlags) {
					if (ImGui::CheckboxFlags(flag.label, &node.samplerID, uint32_t(flag.id))) {
						graph.MarkEdited();
					}
				}
				if (node.samplerID == 0) {
					ImGui::TextDisabled("未選択の場合はDefaultを使います");
				}
				ImGui::TreePop();
			}
		}

		// 実際に使われるグループ数(最初のComputeTriggerの引数で計算)
		auto triggers = LiveTriggers(graph.GetName());
		if (!triggers.empty()) {
			uint32_t groups[3];
			graph.ComputeGroupCount(node, triggers.front()->GetArguments(), groups);
			ImGui::Text("グループ数: %u x %u x %u", groups[0], groups[1], groups[2]);
		}

		if (!node.shaderError.empty()) {
			ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "%s", node.shaderError.c_str());
		} else if (!node.status.empty()) {
			ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "%s", node.status.c_str());
		} else if (node.lastExecuteFrame != UINT32_MAX) {
			ImGui::TextDisabled("最終実行: frame %u", node.lastExecuteFrame);
		}
	}

	void DrawResourceProperties(ComputeGraph& graph, GraphNode& node) {
		GPUBuffer* buffer = PreviewSource(graph, node);
		ImGui::TextDisabled("Resource");
		ImGui::TextWrapped("%s", node.resourceName.empty() ? "(未設定)" : node.resourceName.c_str());
		ImGui::Text("%s", buffer ? BufferTypeText(buffer).c_str() : "見つかりません");

		// つながっているピンの条件で選び直す
		const GraphLink* firstLink = nullptr;
		for (const auto& link : graph.GetLinks()) {
			if (link.from == node.id) { firstLink = &link; break; }
		}
		const GraphPin* pin = nullptr;
		if (firstLink) {
			if (GraphNode* target = graph.GetNode(firstLink->to)) pin = ComputeGraph::FindPin(*target, firstLink->pin);
		}
		ImGui::BeginDisabled(pin == nullptr);
		if (ImGui::Button("Resourceウィンドウで選び直す")) {
			std::string graphName = graph.GetName();
			GraphNodeID id = node.id;
			ResourceRegistry::BeginSelection(pin->request, [graphName, id](const ResourceRef& ref) {
				ComputeGraph* target = ComputeGraphRegistry::Find(graphName);
				GraphNode* resourceNode = target ? target->GetNode(id) : nullptr;
				if (!resourceNode) return;
				resourceNode->resource = ref;
				resourceNode->resourceName = ResourceRegistry::GetPersistentName(ref);
				target->MarkEdited();
				});
		}
		ImGui::EndDisabled();
		if (!pin) {
			ImGui::TextDisabled("(ピンにつなぐと選び直せます)");
		}
	}

	void DrawArgumentProperties(ComputeGraph& graph, GraphNode& node) {
		ImGui::TextDisabled("Argument");
		if (InputString("名前", node.argumentName)) graph.MarkEdited();
		GPUBuffer* buffer = PreviewSource(graph, node);
		ImGui::Text("%s", buffer ? BufferTypeText(buffer).c_str() : "Bindされていません");

		std::set<std::string> names;
		for (ComputeTrigger* trigger : LiveTriggers(graph.GetName())) {
			for (const auto& [name, bound] : trigger->GetArguments().buffers) names.insert(name);
		}
		if (!names.empty()) {
			ImGui::TextDisabled("コードでBindされている名前");
			for (const auto& name : names) {
				if (ImGui::SmallButton(name.c_str())) {
					node.argumentName = name;
					graph.MarkEdited();
				}
				ImGui::SameLine();
			}
			ImGui::NewLine();
		}
	}

	void DrawSidePanel(ComputeGraph* graph) {
		auto& editor = Editor();
		auto& graphs = ComputeGraphRegistry::GetGraphs();

		// グラフの選択(読み込み済み + 保存ファイル)
		if (!editor.fileListLoaded) LoadGraphFileList();
		std::set<std::string> names;
		for (const auto& [name, g] : graphs) names.insert(name);
		for (const auto& name : editor.graphFiles) names.insert(name);

		ImGui::SetNextItemWidth(-1.0f);
		if (ImGui::BeginCombo("##Graph", editor.currentGraph.empty() ? "(グラフを選択)" : editor.currentGraph.c_str())) {
			for (const auto& name : names) {
				const bool live = !LiveTriggers(name).empty();
				std::string label = live ? name : name + "  (未使用)";
				if (ImGui::Selectable(label.c_str(), name == editor.currentGraph)) {
					editor.currentGraph = name;
					editor.selectedNode = kInvalidNodeID;
					editor.selectedLink = -1;
					ComputeGraphRegistry::GetOrLoad(name);
				}
			}
			ImGui::EndCombo();
		}
		if (ImGui::SmallButton("一覧を更新")) {
			LoadGraphFileList();
			LoadShaderList();
		}

		if (!graph) {
			ImGui::TextDisabled("コードでComputeTriggerを作成すると\nここに表示されます");
			return;
		}

		ImGui::Separator();
		if (ImGui::Button(graph->HasUnsavedChanges() ? "保存 *" : "保存", ImVec2(-1.0f, 0.0f))) {
			graph->Save();
			LoadGraphFileList();
		}
		ImGui::TextDisabled("Assets/Binary/%s", graph->GetFilePath().c_str());

		auto triggers = LiveTriggers(graph->GetName());
		if (triggers.empty()) {
			ImGui::TextColored(ImVec4(0.9f, 0.8f, 0.3f, 1.0f), "このグラフのComputeTriggerはありません");
		} else {
			ImGui::Text("ComputeTrigger: %d個", int(triggers.size()));
			uint32_t lastFire = UINT32_MAX;
			for (ComputeTrigger* trigger : triggers) {
				if (trigger->GetLastFireFrame() != UINT32_MAX) lastFire = trigger->GetLastFireFrame();
			}
			if (lastFire != UINT32_MAX) ImGui::TextDisabled("最終Fire: frame %u", lastFire);
		}

		ImGui::Separator();
		GraphNode* node = graph->GetNode(editor.selectedNode);
		if (!node) {
			ImGui::TextDisabled("ノードを選択してください");
			ImGui::Spacing();
			ImGui::TextDisabled("右クリック: ノード追加");
			ImGui::TextDisabled("ピンから空きへドラッグ: リソース選択");
			ImGui::TextDisabled("Resourceウィンドウからドラッグ: 追加");
			ImGui::TextDisabled("中/右ドラッグ: 移動  ホイール: 拡大縮小");
			ImGui::TextDisabled("Delete: 削除  Ctrl+S: 保存");
			return;
		}
		ImGui::PushID(int(node->id));
		switch (node->kind) {
		case GraphNodeKind::Trigger:
			ImGui::TextDisabled("Trigger");
			ImGui::TextWrapped("%s", graph->GetName().c_str());
			ImGui::TextDisabled("コードからFireすると、ここから\n実行順にたどってDispatchします");
			break;
		case GraphNodeKind::Dispatch:
			DrawDispatchProperties(*graph, *node);
			break;
		case GraphNodeKind::Resource:
			DrawResourceProperties(*graph, *node);
			break;
		case GraphNodeKind::Argument:
			DrawArgumentProperties(*graph, *node);
			break;
		}
		ImGui::PopID();
	}

	// ============================================================
	// キャンバス
	// ============================================================

	void DrawContextMenu(ComputeGraph& graph) {
		auto& editor = Editor();
		if (!ImGui::BeginPopup("ComputeGraphMenu")) {
			return;
		}

		switch (editor.menuTarget) {
		case EditorState::MenuTarget::Canvas:
		case EditorState::MenuTarget::ExecDrop:
		{
			if (!editor.shaderListLoaded) LoadShaderList();
			if (ImGui::BeginMenu("Dispatchノードを追加")) {
				std::string selected = DrawShaderMenu(editor.shaderTree);
				if (!selected.empty()) {
					GraphNode& created = CreateDispatchNode(graph, selected, editor.menuPosition);
					editor.selectedNode = created.id;
					// 実行順のピンから引き出した場合は、その後ろに挿入する
					if (editor.menuTarget == EditorState::MenuTarget::ExecDrop) {
						if (GraphNode* from = graph.GetNode(editor.menuNode)) {
							GraphNodeID oldNext = from->next;
							if (graph.SetNext(editor.menuNode, created.id) && oldNext != kInvalidNodeID) {
								graph.SetNext(created.id, oldNext);
							}
						}
					}
				}
				ImGui::EndMenu();
			}
			if (editor.menuTarget == EditorState::MenuTarget::Canvas) {
				if (ImGui::BeginMenu("引数ノードを追加")) {
					std::set<std::string> names;
					for (ComputeTrigger* trigger : LiveTriggers(graph.GetName())) {
						for (const auto& [name, bound] : trigger->GetArguments().buffers) names.insert(name);
					}
					for (const auto& name : names) {
						if (ImGui::MenuItem(name.c_str())) {
							GraphNode& created = graph.AddNode(GraphNodeKind::Argument, editor.menuPosition);
							created.argumentName = name;
							editor.selectedNode = created.id;
						}
					}
					if (!names.empty()) ImGui::Separator();
					ImGui::SetNextItemWidth(160.0f);
					ImGui::InputText("##ArgName", editor.argumentInput, sizeof(editor.argumentInput));
					ImGui::SameLine();
					if (ImGui::Button("追加") && editor.argumentInput[0] != '\0') {
						GraphNode& created = graph.AddNode(GraphNodeKind::Argument, editor.menuPosition);
						created.argumentName = editor.argumentInput;
						editor.selectedNode = created.id;
						editor.argumentInput[0] = '\0';
						ImGui::CloseCurrentPopup();
					}
					ImGui::EndMenu();
				}
				ImGui::MenuItem("リソース: Resourceウィンドウからドラッグ", nullptr, false, false);
			}
			break;
		}
		case EditorState::MenuTarget::Node:
		{
			GraphNode* node = graph.GetNode(editor.menuNode);
			if (!node) break;
			if (node->kind == GraphNodeKind::Dispatch) {
				if (ImGui::MenuItem("有効", nullptr, node->enabled)) {
					node->enabled = !node->enabled;
					graph.MarkEdited();
				}
				if (ImGui::MenuItem("再コンパイル", nullptr, false, !node->shaderName.empty())) {
					RecompileShader(graph, *node);
				}
			}
			if (ImGui::MenuItem("接続をすべて外す")) {
				graph.DisconnectAll(node->id);
				if (node->kind != GraphNodeKind::Resource && node->kind != GraphNodeKind::Argument) {
					graph.SetNext(node->id, kInvalidNodeID);
				}
			}
			if (node->kind != GraphNodeKind::Trigger && ImGui::MenuItem("削除")) {
				graph.RemoveNode(node->id);
				editor.selectedNode = kInvalidNodeID;
			}
			break;
		}
		case EditorState::MenuTarget::Pin:
		{
			const PinHandle& pin = editor.menuPin;
			if (pin.role == PinRole::Input) {
				if (ImGui::MenuItem("Resourceウィンドウで選ぶ")) {
					if (GraphNode* node = graph.GetNode(pin.node)) {
						SelectResourceForPin(graph, pin.node, pin.key, LeftOf(*node));
					}
				}
				if (ImGui::MenuItem("接続を外す")) {
					graph.Disconnect(pin.node, pin.key);
				}
			} else if (pin.role == PinRole::ExecOut) {
				if (ImGui::MenuItem("実行順を外す")) {
					graph.SetNext(pin.node, kInvalidNodeID);
				}
			} else if (pin.role == PinRole::ExecIn) {
				if (ImGui::MenuItem("実行順を外す")) {
					for (auto& [id, other] : graph.GetNodes()) {
						if (other.next == pin.node) graph.SetNext(id, kInvalidNodeID);
					}
				}
			} else {
				if (ImGui::MenuItem("接続をすべて外す")) {
					graph.DisconnectAll(pin.node);
				}
			}
			break;
		}
		}
		ImGui::EndPopup();
	}

	void DrawCanvas(ComputeGraph& graph) {
		auto& editor = Editor();
		ImGuiIO& io = ImGui::GetIO();

		Canvas canvas;
		canvas.origin = ImGui::GetCursorScreenPos();
		canvas.size = ImGui::GetContentRegionAvail();
		canvas.size.x = std::max(canvas.size.x, 50.0f);
		canvas.size.y = std::max(canvas.size.y, 50.0f);
		canvas.scroll = graph.viewScroll;
		canvas.zoom = std::clamp(graph.viewZoom, kMinZoom, kMaxZoom);

		ImGui::InvisibleButton("##ComputeGraphCanvas", canvas.size,
			ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight | ImGuiButtonFlags_MouseButtonMiddle);
		const bool hovered = ImGui::IsItemHovered();
		const bool active = ImGui::IsItemActive();
		const ImVec2 mouse = io.MousePos;
		const Vector2 mouseCanvas = canvas.ToCanvas(mouse);

		// ---- 拡大縮小・移動 ----
		if (hovered && io.MouseWheel != 0.0f) {
			float zoom = std::clamp(canvas.zoom * std::pow(1.1f, io.MouseWheel), kMinZoom, kMaxZoom);
			// マウスの下の点が動かないようにする
			canvas.scroll = { (mouse.x - canvas.origin.x) / zoom - mouseCanvas.x, (mouse.y - canvas.origin.y) / zoom - mouseCanvas.y };
			canvas.zoom = zoom;
		}
		if (active && (ImGui::IsMouseDragging(ImGuiMouseButton_Middle, 0.0f) || ImGui::IsMouseDragging(ImGuiMouseButton_Right, 4.0f))) {
			canvas.scroll.x += io.MouseDelta.x / canvas.zoom;
			canvas.scroll.y += io.MouseDelta.y / canvas.zoom;
			if (ImGui::IsMouseDragging(ImGuiMouseButton_Right, 4.0f)) editor.rightDragged = true;
		}
		graph.viewScroll = canvas.scroll;
		graph.viewZoom = canvas.zoom;

		// ---- レイアウト ----
		std::vector<NodeLayout> layouts;
		for (auto& [id, node] : graph.GetNodes()) {
			layouts.push_back(BuildLayout(graph, node, canvas));
		}
		// 選択中のノードを最前面にする
		std::stable_partition(layouts.begin(), layouts.end(), [&](const NodeLayout& l) { return l.id != editor.selectedNode; });

		auto findPin = [&](const PinHandle& handle) -> const PinLayout* {
			for (const auto& layout : layouts) {
				if (layout.id != handle.node) continue;
				for (const auto& pin : layout.pins) {
					if (pin.handle.role != handle.role) continue;
					if (handle.role == PinRole::Input && !pin.handle.key.SameRegister(handle.key)) continue;
					return &pin;
				}
			}
			return nullptr;
			};

		// ---- ホバー判定 (ピン → ノード → リンク) ----
		const PinLayout* hoveredPin = nullptr;
		GraphNodeID hoveredNode = kInvalidNodeID;
		int hoveredLink = -1;
		if (hovered || active) {
			float bestSq = std::pow(std::max(kPinRadius * canvas.zoom * 1.8f, 7.0f), 2.0f);
			for (const auto& layout : layouts) {
				for (const auto& pin : layout.pins) {
					float d = LengthSq(Sub(pin.screen, mouse));
					if (d < bestSq) { bestSq = d; hoveredPin = &pin; }
				}
			}
			for (auto it = layouts.rbegin(); it != layouts.rend() && !hoveredPin; ++it) {
				if (mouse.x >= it->min.x && mouse.x <= it->max.x && mouse.y >= it->min.y && mouse.y <= it->max.y) {
					hoveredNode = it->id;
					break;
				}
			}
		}

		// リンクの端点
		struct LinkDraw { ImVec2 from, to; ImU32 color; float thickness; int index; };
		std::vector<LinkDraw> linkDraws;
		for (auto& [id, node] : graph.GetNodes()) {
			if ((node.kind == GraphNodeKind::Trigger || node.kind == GraphNodeKind::Dispatch) && node.next != kInvalidNodeID) {
				const PinLayout* a = findPin({ id, PinRole::ExecOut });
				const PinLayout* b = findPin({ node.next, PinRole::ExecIn });
				if (a && b) linkDraws.push_back({ a->screen, b->screen, kExecColor, 3.0f, -1 });
			}
		}
		for (size_t i = 0; i < graph.GetLinks().size(); ++i) {
			const GraphLink& link = graph.GetLinks()[i];
			const PinLayout* a = findPin({ link.from, PinRole::Output });
			const PinLayout* b = findPin({ link.to, PinRole::Input, link.pin });
			if (!a || !b) continue;
			ImU32 color = PinColor(link.pin.type);
			GraphNode* source = graph.GetNode(link.from);
			GraphNode* target = graph.GetNode(link.to);
			const GraphPin* pin = target ? ComputeGraph::FindPin(*target, link.pin) : nullptr;
			if (!pin || !source) {
				color = kErrorColor;
			} else {
				bool known = false;
				if (!IsCompatible(graph, *source, pin->request, &known)) color = kErrorColor;
				else if (!known) color = kUnknownColor;
			}
			linkDraws.push_back({ a->screen, b->screen, color, 2.0f, int(i) });
		}
		if ((hovered || active) && !hoveredPin && hoveredNode == kInvalidNodeID) {
			float bestSq = 36.0f;
			for (const auto& link : linkDraws) {
				if (link.index < 0) continue;
				float d = LinkDistanceSq(link.from, link.to, mouse);
				if (d < bestSq) { bestSq = d; hoveredLink = link.index; }
			}
		}

		// ---- 操作 ----
		if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
			if (hoveredPin) {
				editor.drag = DragKind::Link;
				editor.dragPin = hoveredPin->handle;
			} else if (hoveredNode != kInvalidNodeID) {
				editor.selectedNode = hoveredNode;
				editor.selectedLink = -1;
				editor.drag = DragKind::Node;
				editor.dragNode = hoveredNode;
				const GraphNode* node = graph.GetNode(hoveredNode);
				editor.dragOffset = { mouseCanvas.x - node->position.x, mouseCanvas.y - node->position.y };
				editor.dragMoved = false;
			} else {
				editor.selectedNode = kInvalidNodeID;
				editor.selectedLink = hoveredLink;
			}
		}

		if (editor.drag == DragKind::Node && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
			if (GraphNode* node = graph.GetNode(editor.dragNode)) {
				Vector2 position = { mouseCanvas.x - editor.dragOffset.x, mouseCanvas.y - editor.dragOffset.y };
				if (position.x != node->position.x || position.y != node->position.y) {
					node->position = position;
					editor.dragMoved = true;
				}
			}
		}

		if (editor.drag != DragKind::None && ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
			if (editor.drag == DragKind::Node && editor.dragMoved) {
				graph.MarkEdited();
			} else if (editor.drag == DragKind::Link) {
				if (hoveredPin) {
					TryConnect(graph, editor.dragPin, hoveredPin->handle);
				} else if (editor.dragPin.role == PinRole::Input) {
					// 何もないところで離した → Resourceウィンドウで選ぶ
					SelectResourceForPin(graph, editor.dragPin.node, editor.dragPin.key, { mouseCanvas.x - 200.0f, mouseCanvas.y - 10.0f });
				} else if (editor.dragPin.role == PinRole::ExecOut) {
					// 何もないところで離した → Dispatchノードを追加して後ろにつなぐ
					editor.menuTarget = EditorState::MenuTarget::ExecDrop;
					editor.menuNode = editor.dragPin.node;
					editor.menuPosition = mouseCanvas;
					ImGui::OpenPopup("ComputeGraphMenu");
				}
			}
			editor.drag = DragKind::None;
		}

		// 右クリック(ドラッグしなかった場合)でメニュー
		if (ImGui::IsMouseReleased(ImGuiMouseButton_Right)) {
			if (hovered && !editor.rightDragged) {
				editor.menuPosition = mouseCanvas;
				if (hoveredPin) {
					editor.menuTarget = EditorState::MenuTarget::Pin;
					editor.menuPin = hoveredPin->handle;
				} else if (hoveredNode != kInvalidNodeID) {
					editor.menuTarget = EditorState::MenuTarget::Node;
					editor.menuNode = hoveredNode;
					editor.selectedNode = hoveredNode;
				} else {
					editor.menuTarget = EditorState::MenuTarget::Canvas;
				}
				ImGui::OpenPopup("ComputeGraphMenu");
			}
			editor.rightDragged = false;
		}

		// キー操作 (ウィンドウにフォーカスがあるとき)
		if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && !io.WantTextInput) {
			if (ImGui::IsKeyPressed(ImGuiKey_Delete)) {
				if (editor.selectedNode != kInvalidNodeID) {
					graph.RemoveNode(editor.selectedNode);
					if (!graph.GetNode(editor.selectedNode)) editor.selectedNode = kInvalidNodeID;
				} else if (editor.selectedLink >= 0) {
					graph.RemoveLink(size_t(editor.selectedLink));
					editor.selectedLink = -1;
				}
			}
		}

		// Resourceウィンドウからのドラッグ&ドロップ
		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(ResourceRegistry::kDragDropPayload)) {
				ResourceRef ref = *static_cast<const ResourceRef*>(payload->Data);
				if (hoveredPin && hoveredPin->handle.role == PinRole::Input) {
					// ピンの上に落とした → つなぐ
					GraphNode* target = graph.GetNode(hoveredPin->handle.node);
					const GraphPin* pin = target ? ComputeGraph::FindPin(*target, hoveredPin->handle.key) : nullptr;
					if (pin && !ResourceRegistry::CanBind(ref, pin->request)) {
						ShowMessage("種類・サイズが合わないため割り当てられません: " + pin->request.label);
					} else if (target) {
						GraphNode& created = CreateResourceNode(graph, ref, LeftOf(*target));
						graph.Connect(created.id, target->id, hoveredPin->handle.key);
					}
				} else {
					GraphNode& created = CreateResourceNode(graph, ref, mouseCanvas);
					editor.selectedNode = created.id;
				}
			}
			ImGui::EndDragDropTarget();
		}

		// ---- 描画 ----
		ImDrawList* drawList = ImGui::GetWindowDrawList();
		const ImVec2 canvasMax = Add(canvas.origin, canvas.size);
		drawList->PushClipRect(canvas.origin, canvasMax, true);
		drawList->AddRectFilled(canvas.origin, canvasMax, IM_COL32(25, 25, 28, 255));

		// グリッド
		const float grid = 32.0f * canvas.zoom;
		if (grid > 6.0f) {
			float startX = std::fmod(canvas.scroll.x * canvas.zoom, grid);
			float startY = std::fmod(canvas.scroll.y * canvas.zoom, grid);
			for (float x = startX; x < canvas.size.x; x += grid) {
				drawList->AddLine({ canvas.origin.x + x, canvas.origin.y }, { canvas.origin.x + x, canvasMax.y }, IM_COL32(45, 45, 50, 255));
			}
			for (float y = startY; y < canvas.size.y; y += grid) {
				drawList->AddLine({ canvas.origin.x, canvas.origin.y + y }, { canvasMax.x, canvas.origin.y + y }, IM_COL32(45, 45, 50, 255));
			}
		}

		// リンク
		for (const auto& link : linkDraws) {
			bool emphasized = link.index >= 0 && (link.index == hoveredLink || link.index == editor.selectedLink);
			DrawLinkCurve(drawList, link.from, link.to, emphasized ? IM_COL32(255, 255, 255, 255) : link.color,
				(emphasized ? link.thickness + 1.5f : link.thickness) * std::max(canvas.zoom, 0.6f));
		}

		// ドラッグ中のリンクと、つなげるピンの強調
		std::set<const PinLayout*> highlighted;
		const bool linkDragging = editor.drag == DragKind::Link;
		if (linkDragging) {
			const PinLayout* origin = findPin(editor.dragPin);
			GraphNode* originNode = graph.GetNode(editor.dragPin.node);
			for (const auto& layout : layouts) {
				for (const auto& pin : layout.pins) {
					if (pin.handle.node == editor.dragPin.node || pin.handle.IsOutput() == editor.dragPin.IsOutput()) continue;
					const bool execA = editor.dragPin.role == PinRole::ExecIn || editor.dragPin.role == PinRole::ExecOut;
					const bool execB = pin.handle.role == PinRole::ExecIn || pin.handle.role == PinRole::ExecOut;
					if (execA != execB) continue;
					if (!execA) {
						// リソース → レジスタの相性
						const PinHandle& out = editor.dragPin.IsOutput() ? editor.dragPin : pin.handle;
						const PinHandle& in = editor.dragPin.IsOutput() ? pin.handle : editor.dragPin;
						GraphNode* source = graph.GetNode(out.node);
						GraphNode* target = graph.GetNode(in.node);
						const GraphPin* request = target ? ComputeGraph::FindPin(*target, in.key) : nullptr;
						if (!source || !request || !IsCompatible(graph, *source, request->request)) continue;
					}
					highlighted.insert(&pin);
				}
			}
			if (origin) {
				ImVec2 a = origin->screen;
				ImVec2 b = hoveredPin ? hoveredPin->screen : mouse;
				if (!editor.dragPin.IsOutput()) std::swap(a, b);
				DrawLinkCurve(drawList, a, b, origin->color, 2.0f);
			}
			(void)originNode;
		}

		// ノード
		for (const auto& layout : layouts) {
			DrawNode(drawList, canvas, layout, layout.id == editor.selectedNode, hoveredPin, highlighted, linkDragging);
		}

		// ピンのツールチップ
		if (hoveredPin && !linkDragging && hoveredPin->handle.role == PinRole::Input) {
			if (GraphNode* node = graph.GetNode(hoveredPin->handle.node)) {
				if (const GraphPin* pin = ComputeGraph::FindPin(*node, hoveredPin->handle.key)) {
					ImGui::SetTooltip("%s\nサイズ: %u byte\nドラッグ: つなぐ / 空きに離すとResourceウィンドウで選択", pin->request.label.c_str(), pin->request.size);
				}
			}
		}

		// メッセージ
		if (ImGui::GetTime() - editor.messageTime < 3.0) {
			drawList->AddText({ canvas.origin.x + 8.0f, canvas.origin.y + 8.0f }, IM_COL32(255, 220, 120, 255), editor.message.c_str());
		}
		if (ResourceRegistry::IsSelecting()) {
			drawList->AddText({ canvas.origin.x + 8.0f, canvasMax.y - 24.0f }, IM_COL32(255, 220, 120, 255), "Resourceウィンドウで選択中...");
		}

		drawList->PopClipRect();

		DrawContextMenu(graph);
	}

}
#endif

void SHEngine::ComputeGraphRegistry::DrawImGui() {
#ifdef USE_IMGUI
	auto& editor = Editor();

	// 表示するグラフが未選択なら、生存中のTriggerのものを選ぶ
	if (editor.currentGraph.empty() && !GetTriggers().empty()) {
		editor.currentGraph = GetTriggers().front()->GetName();
	}
	ComputeGraph* graph = editor.currentGraph.empty() ? nullptr : Find(editor.currentGraph);
	if (graph) {
		// シェーダーの変更をすぐ反映させるため、表示中のグラフは毎フレームピンを作り直す
		graph->RefreshAllPins();
	}

	if (!ImGui::Begin("ComputeGraph")) {
		ImGui::End();
		return;
	}

	ImGui::BeginChild("ComputeGraphSide", ImVec2(280.0f, 0.0f), ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX);
	DrawSidePanel(graph);
	ImGui::EndChild();

	ImGui::SameLine();
	ImGui::BeginChild("ComputeGraphCanvas", ImVec2(0.0f, 0.0f), ImGuiChildFlags_None, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoMove);
	if (graph) {
		DrawCanvas(*graph);
		if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S, false)) {
			graph->Save();
			LoadGraphFileList();
		}
	}
	ImGui::EndChild();

	ImGui::End();
#endif
}
