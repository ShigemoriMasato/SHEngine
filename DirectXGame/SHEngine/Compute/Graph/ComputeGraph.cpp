#include "ComputeGraph.h"
#include "ComputeTrigger.h"
#include <Compute/PSO/CSPSOManager.h>
#include <Core/Command/ICommandContext.h>
#include <Tool/Binary/BinaryManager.h>
#include <algorithm>
#include <format>
#include <set>

using namespace SHEngine;

namespace {

	// 保存形式のバージョン。形式を変えたら上げる
	// 1: 初版 / 2: DispatchノードにsamplerIDを追加
	constexpr uint32_t kFormatVersion = 2;
	constexpr uint32_t kMinFormatVersion = 1;
	// Dispatchの1次元あたりの上限
	constexpr uint32_t kMaxGroupCount = 65535;

	struct RegistryState {
		PSO::CSPSOManager* psoManager = nullptr;
		std::map<std::string, std::unique_ptr<ComputeGraph>> graphs;
		std::vector<ComputeTrigger*> triggers;
	};

	RegistryState& State() {
		static RegistryState state;
		return state;
	}

	BufferType ToBufferType(const SlotRequest& request) {
		switch (request.type) {
		case SlotRequest::Type::CBV: return BufferType::CBV;
		case SlotRequest::Type::SRV: return BufferType::SRV;
		case SlotRequest::Type::UAV: return BufferType::UAV;
		case SlotRequest::Type::Texture: return request.writable ? BufferType::UAV : BufferType::SRV;
		}
		return BufferType::CBV;
	}

	uint32_t DivideRoundUp(uint32_t value, uint32_t divisor) {
		divisor = std::max(divisor, 1u);
		return uint32_t((uint64_t(value) + divisor - 1) / divisor);
	}

}

// ============================================================
// ComputeGraph
// ============================================================

ComputeGraph::ComputeGraph(const std::string& name) : name_(name) {
	Reset();
}

void ComputeGraph::Reset() {
	nodes_.clear();
	links_.clear();
	nextNodeID_ = 1;
	viewScroll = { 0.0f, 0.0f };
	viewZoom = 1.0f;
	AddNode(GraphNodeKind::Trigger, { 40.0f, 80.0f });
	needsRefresh_ = true;
}

GraphNode& ComputeGraph::AddNode(GraphNodeKind kind, const Vector2& position) {
	GraphNodeID id = nextNodeID_++;
	GraphNode& node = nodes_[id];
	node.id = id;
	node.kind = kind;
	node.position = position;
	MarkEdited();
	return node;
}

void ComputeGraph::RemoveNode(GraphNodeID id) {
	auto it = nodes_.find(id);
	if (it == nodes_.end() || it->second.kind == GraphNodeKind::Trigger) {
		return;
	}
	// 実行順を詰める(前段 → 削除するノードの次)
	GraphNodeID next = it->second.next;
	for (auto& [otherID, other] : nodes_) {
		if (other.next == id) {
			other.next = next;
		}
	}
	DisconnectAll(id);
	nodes_.erase(it);
	MarkEdited();
}

GraphNode* ComputeGraph::GetNode(GraphNodeID id) {
	auto it = nodes_.find(id);
	return it != nodes_.end() ? &it->second : nullptr;
}

const GraphNode* ComputeGraph::GetNode(GraphNodeID id) const {
	auto it = nodes_.find(id);
	return it != nodes_.end() ? &it->second : nullptr;
}

GraphNode* ComputeGraph::GetTriggerNode() {
	for (auto& [id, node] : nodes_) {
		if (node.kind == GraphNodeKind::Trigger) {
			return &node;
		}
	}
	return nullptr;
}

void ComputeGraph::Connect(GraphNodeID from, GraphNodeID to, const GraphPinKey& pin) {
	Disconnect(to, pin);
	links_.push_back({ from, to, pin });
	MarkEdited();
}

void ComputeGraph::Disconnect(GraphNodeID to, const GraphPinKey& pin) {
	auto removed = std::remove_if(links_.begin(), links_.end(), [&](const GraphLink& link) {
		return link.to == to && link.pin.SameRegister(pin);
		});
	if (removed != links_.end()) {
		links_.erase(removed, links_.end());
		MarkEdited();
	}
}

void ComputeGraph::DisconnectAll(GraphNodeID id) {
	auto removed = std::remove_if(links_.begin(), links_.end(), [&](const GraphLink& link) {
		return link.from == id || link.to == id;
		});
	if (removed != links_.end()) {
		links_.erase(removed, links_.end());
		MarkEdited();
	}
}

void ComputeGraph::RemoveLink(size_t index) {
	if (index < links_.size()) {
		links_.erase(links_.begin() + index);
		MarkEdited();
	}
}

bool ComputeGraph::SetNext(GraphNodeID from, GraphNodeID to) {
	GraphNode* fromNode = GetNode(from);
	if (!fromNode || (fromNode->kind != GraphNodeKind::Trigger && fromNode->kind != GraphNodeKind::Dispatch)) {
		return false;
	}
	if (to != kInvalidNodeID) {
		GraphNode* toNode = GetNode(to);
		if (!toNode || toNode->kind != GraphNodeKind::Dispatch || to == from) {
			return false;
		}
		// toから先をたどってfromに戻ってくるなら循環する
		std::set<GraphNodeID> visited;
		for (GraphNodeID cursor = to; cursor != kInvalidNodeID && visited.insert(cursor).second;) {
			if (cursor == from) {
				return false;
			}
			const GraphNode* node = GetNode(cursor);
			cursor = node ? node->next : kInvalidNodeID;
		}
		// 前段は1つだけにする
		for (auto& [id, node] : nodes_) {
			if (node.next == to) {
				node.next = kInvalidNodeID;
			}
		}
	}
	fromNode->next = to;
	MarkEdited();
	return true;
}

const GraphLink* ComputeGraph::FindLink(GraphNodeID to, const GraphPinKey& pin) const {
	for (const auto& link : links_) {
		if (link.to == to && link.pin.SameRegister(pin)) {
			return &link;
		}
	}
	return nullptr;
}

const GraphPin* ComputeGraph::FindPin(const GraphNode& node, const GraphPinKey& key) {
	for (const auto& pin : node.pins) {
		if (pin.key.SameRegister(key)) {
			return &pin;
		}
	}
	for (const auto& pin : node.pins) {
		if (!key.name.empty() && pin.key.SameName(key)) {
			return &pin;
		}
	}
	return nullptr;
}

std::vector<GraphNodeID> ComputeGraph::GetExecutionOrder() const {
	std::vector<GraphNodeID> order;
	const GraphNode* trigger = nullptr;
	for (const auto& [id, node] : nodes_) {
		if (node.kind == GraphNodeKind::Trigger) {
			trigger = &node;
			break;
		}
	}
	if (!trigger) {
		return order;
	}
	std::set<GraphNodeID> visited;
	for (GraphNodeID cursor = trigger->next; cursor != kInvalidNodeID && visited.insert(cursor).second;) {
		const GraphNode* node = GetNode(cursor);
		if (!node || node->kind != GraphNodeKind::Dispatch) {
			break;
		}
		order.push_back(cursor);
		cursor = node->next;
	}
	return order;
}

bool ComputeGraph::HasNoDispatch() const {
	for (const auto& [id, node] : nodes_) {
		if (node.kind == GraphNodeKind::Dispatch) {
			return false;
		}
	}
	return true;
}

GraphNodeID ComputeGraph::AppendDispatch(const std::string& shaderName, const std::string& countArgument) {
	// 実行順の最後を探す
	GraphNode* last = GetTriggerNode();
	std::vector<GraphNodeID> order = GetExecutionOrder();
	if (!order.empty()) {
		last = GetNode(order.back());
	}
	const Vector2 position = last ? Vector2{ last->position.x + 320.0f, last->position.y } : Vector2{ 360.0f, 80.0f };

	GraphNode& node = AddNode(GraphNodeKind::Dispatch, position);
	node.shaderName = shaderName;
	if (!countArgument.empty()) {
		node.dispatchMode = DispatchMode::ArgumentCount;
		node.countArgument = countArgument;
	}
	const GraphNodeID id = node.id;
	if (last) {
		SetNext(last->id, id);
	}
	return id;
}

void ComputeGraph::ConnectArgument(GraphNodeID dispatch, SlotRequest::Type type, uint32_t registerIndex, const std::string& argumentName) {
	GraphNode* target = GetNode(dispatch);
	if (!target) {
		return;
	}

	GraphNodeID source = kInvalidNodeID;
	int argumentCount = 0;
	for (const auto& [id, node] : nodes_) {
		if (node.kind != GraphNodeKind::Argument) continue;
		++argumentCount;
		if (node.argumentName == argumentName) {
			source = id;
		}
	}
	if (source == kInvalidNodeID) {
		// 引数ノードはTriggerの下に縦に並べる
		const GraphNode* trigger = GetTriggerNode();
		Vector2 position = trigger ? Vector2{ trigger->position.x, trigger->position.y + 120.0f + 70.0f * float(argumentCount) } : Vector2{ 40.0f, 200.0f };
		GraphNode& node = AddNode(GraphNodeKind::Argument, position);
		node.argumentName = argumentName;
		source = node.id;
	}
	// 変数名はピンを作るときに付け替えられる(レジスタで照合する)
	Connect(source, dispatch, { type, registerIndex, "" });
}

void ComputeGraph::MarkEdited() {
	unsaved_ = true;
	needsRefresh_ = true;
}

// ============================================================
// ピン
// ============================================================

void ComputeGraph::RefreshPins(GraphNode& node) {
	if (node.kind != GraphNodeKind::Dispatch) {
		return;
	}

	node.pins.clear();
	node.useBindlessTexture = false;
	node.useSampler = false;
	node.shaderValid = false;
	node.shaderError.clear();
	node.numThreads[0] = node.numThreads[1] = node.numThreads[2] = 1;

	if (node.shaderName.empty()) {
		node.shaderError = "シェーダーが設定されていません";
		return;
	}
	PSO::CSPSOManager* psoManager = ComputeGraphRegistry::GetPSOManager();
	const ShaderReflectionData* reflection = psoManager ? psoManager->GetReflection(node.shaderName) : nullptr;
	if (!reflection) {
		node.shaderError = "シェーダーを読み込めません: " + node.shaderName;
		return;
	}
	node.shaderValid = true;
	for (int i = 0; i < 3; ++i) {
		node.numThreads[i] = std::max(reflection->threadGroupSize[i], 1u);
	}

	auto addBuffers = [&](const std::vector<ShaderBufferBinding>& bindings, BufferType bufferType) {
		for (const auto& binding : bindings) {
			if (binding.space != 0) {
				// ルートシグネチャが合わなくなるので、このノードは実行しない
				node.shaderValid = false;
				node.shaderError = std::format("space{}のレジスタ({})には対応していません", binding.space, binding.name);
				continue;
			}
			GraphPin pin;
			pin.request = SlotRequest::FromBuffer(bufferType, binding);
			pin.key = { pin.request.type, binding.registerIndex, binding.name };
			node.pins.push_back(pin);
		}
		};
	addBuffers(reflection->cbvs, BufferType::CBV);
	addBuffers(reflection->srvs, BufferType::SRV);
	addBuffers(reflection->uavs, BufferType::UAV);

	for (const auto& binding : reflection->textures) {
		// 上限なしの配列 (Texture2D tex[]) はバインドレス(t8~)で参照する
		if (binding.count == 0) {
			node.useBindlessTexture = true;
			continue;
		}
		if (binding.space != 0 || binding.count != 1) {
			node.shaderValid = false;
			node.shaderError = std::format("テクスチャ配列・space指定({})には対応していません", binding.name);
			continue;
		}
		GraphPin pin;
		pin.request = SlotRequest::FromTexture(binding);
		pin.key = { pin.request.type, binding.registerIndex, binding.name };
		node.pins.push_back(pin);
	}
	node.useSampler = !reflection->samplers.empty();

	// リンクのキーを新しいピンに付け替える(変数名だけ変わった、レジスタだけ変わったなど)
	for (auto& link : links_) {
		if (link.to != node.id) {
			continue;
		}
		if (const GraphPin* pin = FindPin(node, link.pin)) {
			link.pin = pin->key;
		}
	}
	if (node.dispatchMode == DispatchMode::ResourceCount) {
		if (const GraphPin* pin = FindPin(node, node.countPin)) {
			node.countPin = pin->key;
		}
	}
}

void ComputeGraph::RefreshAllPins() {
	for (auto& [id, node] : nodes_) {
		RefreshPins(node);
	}
	needsRefresh_ = false;
}

// ============================================================
// 実行
// ============================================================

void ComputeGraph::SetStatus(GraphNode& node, const std::string& status) {
	if (node.status != status && !status.empty()) {
		logger_->warn("ComputeGraph \"{}\" ノード{} ({}): {}", name_, node.id, node.shaderName, status);
	}
	node.status = status;
}

GPUBuffer* ComputeGraph::ResolveSource(GraphNode& node, const ComputeArguments& arguments) {
	if (node.kind == GraphNodeKind::Argument) {
		auto it = arguments.buffers.find(node.argumentName);
		return it != arguments.buffers.end() ? it->second : nullptr;
	}
	if (node.kind != GraphNodeKind::Resource) {
		return nullptr;
	}

	GPUBuffer* buffer = ResourceRegistry::Resolve(node.resource);
	if (buffer) {
		return buffer;
	}
	// 削除された・まだ作られていない・読み込んだ直後でIDがない → 名前で探し直す(リソースが増減したときだけ)
	const uint32_t generation = ResourceRegistry::GetGeneration();
	if (node.resolveGeneration != generation && !node.resourceName.empty()) {
		node.resolveGeneration = generation;
		ResourceRef found = ResourceRegistry::FindByPersistentName(node.resource.source, node.resourceName);
		if (!found.IsNone()) {
			node.resource = found;
			buffer = ResourceRegistry::Resolve(found);
		}
	}
	return buffer;
}

GPUBuffer* ComputeGraph::ResolvePin(GraphNode& node, const GraphPinKey& key, bool writable, const ComputeArguments& arguments, GraphNode** outSource) {
	if (outSource) *outSource = nullptr;
	if (const GraphLink* link = FindLink(node.id, key)) {
		GraphNode* source = GetNode(link->from);
		if (outSource) *outSource = source;
		return source ? ResolveSource(*source, arguments) : nullptr;
	}
	const DefaultBinding* binding = arguments.FindDefault(key, writable);
	return binding ? binding->buffer : nullptr;
}

void ComputeGraph::ComputeGroupCount(GraphNode& node, const ComputeArguments& arguments, uint32_t outGroups[3]) {
	outGroups[0] = outGroups[1] = outGroups[2] = 0;

	uint32_t threads[3] = { 0, 1, 1 };
	switch (node.dispatchMode) {
	case DispatchMode::Groups:
		for (int i = 0; i < 3; ++i) {
			outGroups[i] = node.dispatchSize[i];
		}
		break;
	case DispatchMode::Threads:
		for (int i = 0; i < 3; ++i) {
			threads[i] = node.dispatchSize[i];
		}
		break;
	case DispatchMode::ResourceCount:
	{
		const GraphPin* pin = FindPin(node, node.countPin);
		if (GPUBuffer* buffer = ResolvePin(node, node.countPin, pin && pin->request.writable, arguments)) {
			threads[0] = buffer->GetNum();
		}
		break;
	}
	case DispatchMode::ArgumentCount:
	{
		auto it = arguments.counts.find(node.countArgument);
		if (it != arguments.counts.end()) {
			threads[0] = it->second;
		}
		break;
	}
	}

	if (node.dispatchMode != DispatchMode::Groups) {
		for (int i = 0; i < 3; ++i) {
			outGroups[i] = DivideRoundUp(threads[i], node.numThreads[i]);
		}
	}
	for (int i = 0; i < 3; ++i) {
		outGroups[i] = std::min(outGroups[i], kMaxGroupCount);
	}
}

void ComputeGraph::Execute(ICommandContext* context, const ComputeArguments& arguments) {
	if (needsRefresh_) {
		RefreshAllPins();
	}

	for (GraphNodeID id : GetExecutionOrder()) {
		GraphNode& node = nodes_.at(id);
		if (!node.enabled) {
			continue;
		}
		if (!node.shaderValid) {
			SetStatus(node, node.shaderError);
			continue;
		}

		if (!node.computeObject) {
			node.computeObject = std::make_unique<ComputeObject>(name_ + "/" + std::to_string(node.id));
			node.computeObject->Initialize();
		}
		ComputeObject* object = node.computeObject.get();
		object->ResetGPUBuffers();
		object->SetShader(node.shaderName);

		// ピンの割り当て(リンクがなければ既定のバッファ)
		std::string problem;
		for (const auto& pin : node.pins) {
			GraphNode* source = nullptr;
			GPUBuffer* buffer = ResolvePin(node, pin.key, pin.request.writable, arguments, &source);
			if (!buffer) {
				if (!source) {
					problem = "未接続: " + pin.request.label;
				} else {
					problem = source->kind == GraphNodeKind::Argument
						? std::format("引数 \"{}\" がBindされていません ({})", source->argumentName, pin.request.label)
						: std::format("リソース \"{}\" が見つかりません ({})", source->resourceName, pin.request.label);
				}
				break;
			}
			if (!ResourceRegistry::CanBindBuffer(buffer, pin.request)) {
				problem = "割り当てられない種類・サイズです: " + pin.request.label;
				break;
			}
			object->SetGPUBuffer(ToBufferType(pin.request), int(pin.key.registerIndex), buffer);
		}
		if (!problem.empty()) {
			SetStatus(node, problem);
			continue;
		}

		uint32_t groups[3];
		ComputeGroupCount(node, arguments, groups);
		if (groups[0] == 0 || groups[1] == 0 || groups[2] == 0) {
			// 数が0のときは実行しないだけ(エラーではない)
			node.status.clear();
			continue;
		}

		object->SetUseTexture(node.useBindlessTexture);
		uint32_t samplerID = node.samplerID != 0 ? node.samplerID : uint32_t(PSO::SamplerID::Default);
		object->SetSamplerID((node.useSampler || node.useBindlessTexture) ? samplerID : 0u);
		object->SetThreadGroupSize(int(groups[0]), int(groups[1]), int(groups[2]));
		object->Execute(context);

		node.status.clear();
		node.lastExecuteFrame = GPUBuffer::GetCurrentFrame();
	}
}

// ============================================================
// 保存
// ============================================================

bool ComputeGraph::Load() {
	BinaryManager bin;
	if (!bin.Boot(GetFilePath())) {
		return false;
	}

	uint32_t version = 0;
	bin.Reverse(version);
	if (version < kMinFormatVersion || version > kFormatVersion) {
		logger_->error("ComputeGraph \"{}\": 保存形式のバージョンが違うため読み込めません ({} != {})", name_, version, kFormatVersion);
		return false;
	}

	nodes_.clear();
	links_.clear();

	bin.Reverse(nextNodeID_);
	bin.Reverse(viewScroll);
	bin.Reverse(viewZoom);

	uint32_t nodeCount = 0;
	bin.Reverse(nodeCount);
	for (uint32_t i = 0; i < nodeCount && !bin.IsFailed(); ++i) {
		uint8_t kind = 0;
		GraphNodeID id = kInvalidNodeID;
		bin.Reverse(kind);
		bin.Reverse(id);
		GraphNode& node = nodes_[id];
		node.id = id;
		node.kind = GraphNodeKind(kind);
		bin.Reverse(node.position);
		bin.Reverse(node.next);
		bin.Reverse(node.shaderName);
		bin.Reverse(node.enabled);
		uint8_t mode = 0;
		bin.Reverse(mode);
		node.dispatchMode = DispatchMode(mode);
		for (int axis = 0; axis < 3; ++axis) {
			bin.Reverse(node.dispatchSize[axis]);
		}
		uint8_t countPinType = 0;
		bin.Reverse(countPinType);
		node.countPin.type = SlotRequest::Type(countPinType);
		bin.Reverse(node.countPin.registerIndex);
		bin.Reverse(node.countPin.name);
		bin.Reverse(node.countArgument);
		uint8_t source = 0;
		bin.Reverse(source);
		node.resource = {};
		node.resource.source = ResourceRef::Source(source);
		bin.Reverse(node.resourceName);
		bin.Reverse(node.argumentName);
		if (version >= 2) {
			bin.Reverse(node.samplerID);
		}
	}

	uint32_t linkCount = 0;
	bin.Reverse(linkCount);
	for (uint32_t i = 0; i < linkCount && !bin.IsFailed(); ++i) {
		GraphLink link;
		uint8_t type = 0;
		bin.Reverse(link.from);
		bin.Reverse(link.to);
		bin.Reverse(type);
		link.pin.type = SlotRequest::Type(type);
		bin.Reverse(link.pin.registerIndex);
		bin.Reverse(link.pin.name);
		links_.push_back(link);
	}

	if (bin.IsFailed() || !GetTriggerNode()) {
		logger_->error("ComputeGraph \"{}\": ファイルが壊れているため読み込めません ({})", name_, GetFilePath());
		Reset();
		unsaved_ = false;
		return false;
	}

	// Resourceノードは名前で探す(IDは起動ごとに変わる)
	for (auto& [id, node] : nodes_) {
		if (node.kind == GraphNodeKind::Resource) {
			node.resource.id = -1;
			node.resolveGeneration = UINT32_MAX;
		}
	}

	unsaved_ = false;
	needsRefresh_ = true;
	return true;
}

void ComputeGraph::Save() {
	BinaryManager bin;
	bin.Register(kFormatVersion);
	bin.Register(nextNodeID_);
	bin.Register(viewScroll);
	bin.Register(viewZoom);

	bin.Register(uint32_t(nodes_.size()));
	for (auto& [id, node] : nodes_) {
		// 名前が変わっていても今の名前で保存する
		if (node.kind == GraphNodeKind::Resource) {
			std::string current = ResourceRegistry::GetPersistentName(node.resource);
			if (!current.empty()) {
				node.resourceName = current;
			}
		}
		bin.Register(uint8_t(node.kind));
		bin.Register(node.id);
		bin.Register(node.position);
		bin.Register(node.next);
		bin.Register(node.shaderName);
		bin.Register(node.enabled);
		bin.Register(uint8_t(node.dispatchMode));
		for (int axis = 0; axis < 3; ++axis) {
			bin.Register(node.dispatchSize[axis]);
		}
		bin.Register(uint8_t(node.countPin.type));
		bin.Register(node.countPin.registerIndex);
		bin.Register(node.countPin.name);
		bin.Register(node.countArgument);
		bin.Register(uint8_t(node.resource.source));
		bin.Register(node.resourceName);
		bin.Register(node.argumentName);
		bin.Register(node.samplerID);
	}

	bin.Register(uint32_t(links_.size()));
	for (const auto& link : links_) {
		bin.Register(link.from);
		bin.Register(link.to);
		bin.Register(uint8_t(link.pin.type));
		bin.Register(link.pin.registerIndex);
		bin.Register(link.pin.name);
	}

	bin.Write(GetFilePath());
	unsaved_ = false;
	logger_->info("ComputeGraph \"{}\" を保存しました ({})", name_, GetFilePath());
}

// ============================================================
// ComputeGraphRegistry
// ============================================================

void ComputeGraphRegistry::StaticInitialize(PSO::CSPSOManager* psoManager) {
	State().psoManager = psoManager;
}

PSO::CSPSOManager* ComputeGraphRegistry::GetPSOManager() {
	return State().psoManager;
}

ComputeGraph* ComputeGraphRegistry::GetOrLoad(const std::string& name) {
	auto& graphs = State().graphs;
	auto it = graphs.find(name);
	if (it != graphs.end()) {
		return it->second.get();
	}
	auto graph = std::make_unique<ComputeGraph>(name);
	if (!graph->Load()) {
		// ファイルがない場合は、Triggerノードだけのグラフ(何も実行しない)から始める
		GetLogger("Engine")->info("ComputeGraph \"{}\": 保存データがないため新しく作成しました", name);
	}
	ComputeGraph* result = graph.get();
	graphs[name] = std::move(graph);
	return result;
}

ComputeGraph* ComputeGraphRegistry::Find(const std::string& name) {
	auto& graphs = State().graphs;
	auto it = graphs.find(name);
	return it != graphs.end() ? it->second.get() : nullptr;
}

std::map<std::string, std::unique_ptr<ComputeGraph>>& ComputeGraphRegistry::GetGraphs() {
	return State().graphs;
}

void ComputeGraphRegistry::AddTrigger(ComputeTrigger* trigger) {
	State().triggers.push_back(trigger);
}

void ComputeGraphRegistry::RemoveTrigger(ComputeTrigger* trigger) {
	auto& triggers = State().triggers;
	triggers.erase(std::remove(triggers.begin(), triggers.end(), trigger), triggers.end());
}

const std::vector<ComputeTrigger*>& ComputeGraphRegistry::GetTriggers() {
	return State().triggers;
}

// ============================================================
// ComputeTrigger
// ============================================================

ComputeTrigger::ComputeTrigger(const std::string& name) : name_(name) {
	graph_ = ComputeGraphRegistry::GetOrLoad(name);
	ComputeGraphRegistry::AddTrigger(this);
}

ComputeTrigger::~ComputeTrigger() {
	ComputeGraphRegistry::RemoveTrigger(this);
}

void ComputeTrigger::Bind(const std::string& argumentName, GPUBuffer* buffer) {
	if (buffer) {
		arguments_.buffers[argumentName] = buffer;
	} else {
		arguments_.buffers.erase(argumentName);
	}
}

void ComputeTrigger::SetCount(const std::string& countName, uint32_t count) {
	arguments_.counts[countName] = count;
}

void ComputeTrigger::BindDefault(BufferType bufferType, uint32_t registerIndex, GPUBuffer* buffer, const std::string& label) {
	SlotRequest::Type type = SlotRequest::Type::CBV;
	if (bufferType == BufferType::SRV) {
		type = SlotRequest::Type::SRV;
	} else if (bufferType == BufferType::UAV) {
		type = SlotRequest::Type::UAV;
	} else if (bufferType != BufferType::CBV) {
		GetLogger("Engine")->error("ComputeTrigger({})::BindDefault: BufferTypeはCBV/SRV/UAVのどれか1つを指定してください", name_);
		return;
	}
	if (buffer) {
		arguments_.defaults[{ type, registerIndex }] = { buffer, label.empty() ? buffer->GetName() : label };
	} else {
		arguments_.defaults.erase({ type, registerIndex });
	}
}

void ComputeTrigger::Fire(ICommandContext* context) {
	if (!graph_ || !context) {
		return;
	}
	lastFireFrame_ = GPUBuffer::GetCurrentFrame();
	graph_->Execute(context, arguments_);
}
