#pragma once
#include <Render/Buffer/ResourceRegistry.h>
#include <Compute/ComputeObject.h>
#include <Utility/Vector.h>
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace SHEngine {

	class ICommandContext;
	class ComputeTrigger;
	namespace PSO { class CSPSOManager; }

	using GraphNodeID = uint32_t;
	inline constexpr GraphNodeID kInvalidNodeID = 0;

	// @brief ComputeShaderのレジスタ1つ分(ノードの入力ピン)を表すキー
	// @note シェーダーを書き換えても同じ割り当てを保てるよう、(種類, レジスタ番号) → (種類, 変数名) の順で照合する
	struct GraphPinKey {
		SlotRequest::Type type = SlotRequest::Type::CBV;
		uint32_t registerIndex = 0;
		std::string name;

		bool SameRegister(const GraphPinKey& other) const { return type == other.type && registerIndex == other.registerIndex; }
		bool SameName(const GraphPinKey& other) const { return type == other.type && name == other.name; }
	};

	// @brief リフレクションから作られる、Dispatchノードの入力ピン
	struct GraphPin {
		GraphPinKey key;
		SlotRequest request;
	};

	enum class GraphNodeKind : uint8_t {
		Trigger,	// 実行の起点。グラフに1つ。コードのComputeTriggerと対応する
		Dispatch,	// ComputeShader 1本 = Dispatch 1回
		Resource,	// ResourceRegistryに登録されたバッファ・テクスチャ
		Argument,	// コードからComputeTrigger::Bindで渡されるバッファ
	};

	enum class DispatchMode : uint8_t {
		Threads,		// 総スレッド数を指定 (グループ数はnumthreadsで割って切り上げ)
		Groups,			// スレッドグループ数を直接指定
		ResourceCount,	// 指定したピンにつながったリソースの要素数をXのスレッド数にする
		ArgumentCount,	// ComputeTrigger::SetCountで渡された数をXのスレッド数にする
	};

	// @brief ノード1つ分。種類によって使うメンバが異なる
	struct GraphNode {
		GraphNodeID id = kInvalidNodeID;
		GraphNodeKind kind = GraphNodeKind::Dispatch;
		Vector2 position;

		// ---- Trigger / Dispatch ----
		GraphNodeID next = kInvalidNodeID;	// 次に実行するDispatchノード

		// ---- Dispatch ----
		std::string shaderName;				// Assets/Shader/Compute/以下のファイル名 (例: "Particle/Update.CS.hlsl")
		bool enabled = true;
		DispatchMode dispatchMode = DispatchMode::Threads;
		uint32_t dispatchSize[3] = { 1, 1, 1 };
		GraphPinKey countPin;				// ResourceCount: 要素数を使うピン
		std::string countArgument;			// ArgumentCount: 数の名前
		uint32_t samplerID = uint32_t(PSO::SamplerID::Default);	// シェーダーがサンプラーを使う場合のSamplerID(ORの組み合わせ)

		// ---- Resource ----
		ResourceRef resource;
		std::string resourceName;			// 保存用の名前 (ResourceRegistry::GetPersistentName)

		// ---- Argument ----
		std::string argumentName;

		// ---- 実行時の情報 (保存しない) ----
		std::vector<GraphPin> pins;			// リフレクションから作ったピン (register昇順)
		uint32_t numThreads[3] = { 1, 1, 1 };
		bool useBindlessTexture = false;	// 上限なしのテクスチャ配列(t8~)を使うか
		bool useSampler = false;
		bool shaderValid = false;
		std::string shaderError;			// リフレクションで見つかった、実行できない理由
		std::string status;					// 最後に実行したときの問題 (なければ空)
		uint32_t lastExecuteFrame = UINT32_MAX;
		uint32_t resolveGeneration = UINT32_MAX;	// Resource: 名前から再検索したときのResourceRegistryの世代
		std::unique_ptr<ComputeObject> computeObject;
	};

	// @brief Resource/Argumentノードの出力 → Dispatchノードの入力ピン
	struct GraphLink {
		GraphNodeID from = kInvalidNodeID;
		GraphNodeID to = kInvalidNodeID;
		GraphPinKey pin;
	};

	// @brief 何もつながっていないピンに使うバッファ(ComputeTrigger::BindDefault)
	struct DefaultBinding {
		GPUBuffer* buffer = nullptr;
		std::string label;	// ノード画面に表示する名前
	};

	// @brief ComputeTriggerからグラフへ渡す値
	struct ComputeArguments {
		std::unordered_map<std::string, GPUBuffer*> buffers;
		std::unordered_map<std::string, uint32_t> counts;
		// (ピンの種類, レジスタ番号) → 既定のバッファ
		std::map<std::pair<SlotRequest::Type, uint32_t>, DefaultBinding> defaults;

		// @brief ピンの既定のバッファ。テクスチャのピンはSRV/UAVの既定も見る
		const DefaultBinding* FindDefault(const GraphPinKey& key, bool writable = false) const {
			auto it = defaults.find({ key.type, key.registerIndex });
			if (it == defaults.end() && key.type == SlotRequest::Type::Texture) {
				it = defaults.find({ writable ? SlotRequest::Type::UAV : SlotRequest::Type::SRV, key.registerIndex });
			}
			return it != defaults.end() && it->second.buffer ? &it->second : nullptr;
		}
	};

	// @brief ComputeShaderをどの順番で、どのリソースを割り当てて実行するかを表すグラフ。
	// @note ComputeTriggerの名前ごとに1つ作られ、"Assets/Binary/ComputeGraph/<名前>.bin" に保存される。
	//       Releaseでは読み込んだ内容を実行するだけで、編集・保存は行わない。
	class ComputeGraph {
	public:

		explicit ComputeGraph(const std::string& name);

		const std::string& GetName() const { return name_; }

		// ---- ノード・リンクの編集 ----

		GraphNode& AddNode(GraphNodeKind kind, const Vector2& position);
		// @brief ノードと、それにつながるリンクを削除する。Triggerノードは削除できない
		void RemoveNode(GraphNodeID id);
		GraphNode* GetNode(GraphNodeID id);
		const GraphNode* GetNode(GraphNodeID id) const;
		GraphNode* GetTriggerNode();
		std::map<GraphNodeID, GraphNode>& GetNodes() { return nodes_; }
		const std::vector<GraphLink>& GetLinks() const { return links_; }

		// @brief リソースをピンにつなぐ。既にそのピンにつながっているリンクは置き換える
		void Connect(GraphNodeID from, GraphNodeID to, const GraphPinKey& pin);
		// @brief ピンにつながっているリンクを外す
		void Disconnect(GraphNodeID to, const GraphPinKey& pin);
		// @brief ノードにつながっているリンクをすべて外す
		void DisconnectAll(GraphNodeID id);
		void RemoveLink(size_t index);
		// @brief 実行順をつなぐ。循環する場合は失敗する。toの他の前段は外れる
		bool SetNext(GraphNodeID from, GraphNodeID to);
		// @brief ピンにつながっているリンク
		const GraphLink* FindLink(GraphNodeID to, const GraphPinKey& pin) const;
		// @brief ノードのピンからキーに対応するものを探す
		static const GraphPin* FindPin(const GraphNode& node, const GraphPinKey& key);

		// @brief Triggerから実行順にたどったDispatchノード
		std::vector<GraphNodeID> GetExecutionOrder() const;

		// ---- コードから初期状態を作るためのヘルパー ----
		// 保存データがない(Dispatchノードがない)ときに、コードで既定のグラフを組み立てる用途。
		//   if (graph->HasNoDispatch()) {
		//       GraphNodeID id = graph->AppendDispatch("Particle/Update.CS.hlsl", "Particles");
		//       graph->ConnectArgument(id, SlotRequest::Type::UAV, 4, "Velocities");
		//   }

		// @brief Dispatchノードが1つもないか
		bool HasNoDispatch() const;
		// @brief 実行順の最後にDispatchノードを追加する
		// @param countArgument 空でなければ、Dispatch数をその引数の数(ArgumentCount)にする
		GraphNodeID AppendDispatch(const std::string& shaderName, const std::string& countArgument = "");
		// @brief 引数ノード(同じ名前があれば再利用)をDispatchノードのピンにつなぐ
		void ConnectArgument(GraphNodeID dispatch, SlotRequest::Type type, uint32_t registerIndex, const std::string& argumentName);

		// @brief 編集したことを記録する(保存が必要になり、次の実行前にピンを作り直す)
		void MarkEdited();
		bool HasUnsavedChanges() const { return unsaved_; }

		// ---- ピン ----

		// @brief シェーダーのリフレクションからピンを作り直し、リンクのキーを付け替える
		void RefreshPins(GraphNode& node);
		void RefreshAllPins();
		// @brief 次の実行前にピンを作り直す(シェーダーを再コンパイルしたときなど。保存は不要)
		void RequestRefresh() { needsRefresh_ = true; }

		// ---- 実行 ----

		// @brief Triggerから順にComputeShaderを実行する
		void Execute(ICommandContext* context, const ComputeArguments& arguments);
		// @brief Resource/Argumentノードが指すバッファ
		GPUBuffer* ResolveSource(GraphNode& node, const ComputeArguments& arguments);
		// @brief ピンに割り当てるバッファ。リンクがあればその先、なければ既定のバッファ
		// @param outSource リンク先のノード(既定を使った場合はnullptr)
		GPUBuffer* ResolvePin(GraphNode& node, const GraphPinKey& key, bool writable, const ComputeArguments& arguments, GraphNode** outSource = nullptr);
		// @brief Dispatchするスレッドグループ数を計算する。0なら実行しない
		void ComputeGroupCount(GraphNode& node, const ComputeArguments& arguments, uint32_t outGroups[3]);

		// ---- 保存 ----

		// @return ファイルがあり、読み込めたらtrue
		bool Load();
		void Save();
		std::string GetFilePath() const { return "ComputeGraph/" + name_ + ".bin"; }

		// ---- エディタの表示状態 (保存する) ----
		Vector2 viewScroll;
		float viewZoom = 1.0f;

	private:

		// @brief Triggerノードだけの状態にする
		void Reset();
		void SetStatus(GraphNode& node, const std::string& status);

		std::string name_;
		std::map<GraphNodeID, GraphNode> nodes_;
		std::vector<GraphLink> links_;
		GraphNodeID nextNodeID_ = 1;

		bool unsaved_ = false;
		bool needsRefresh_ = true;

		Logger logger_ = GetLogger("Engine");
	};

	// @brief 全ComputeGraphと、生存中のComputeTriggerを管理する
	class ComputeGraphRegistry {
	public:

		ComputeGraphRegistry() = delete;

		static void StaticInitialize(PSO::CSPSOManager* psoManager);
		static PSO::CSPSOManager* GetPSOManager();

		// @brief 名前のグラフを取得する。なければファイルから読み込み、ファイルもなければ新しく作る
		static ComputeGraph* GetOrLoad(const std::string& name);
		static ComputeGraph* Find(const std::string& name);
		static std::map<std::string, std::unique_ptr<ComputeGraph>>& GetGraphs();

		// ---- ComputeTrigger (コンストラクタ・デストラクタから呼ばれる) ----
		static void AddTrigger(ComputeTrigger* trigger);
		static void RemoveTrigger(ComputeTrigger* trigger);
		static const std::vector<ComputeTrigger*>& GetTriggers();

		// @brief ノード編集画面 ("ComputeGraph" ウィンドウ)。USE_IMGUIが無効なら何もしない
		static void DrawImGui();
	};

}
