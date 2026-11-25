#include "ResourceRegistry.h"
#include <Assets/Texture/TextureManager.h>
#include <algorithm>
#include <format>
#include <map>
#include <unordered_map>

#ifdef USE_IMGUI
#include <imgui/imgui.h>
#endif

using namespace SHEngine;

namespace {

	// 静的な状態。関数内staticにすることで、最初のBufferContainerより先に作られ、後に破棄される
	struct RegistryState {
		TextureManager* textureManager = nullptr;

		GroupID nextGroupID = 1;
		ResourceID nextResourceID = 1;

		// 表示順を安定させるためstd::mapにする(IDは作成順)
		std::map<GroupID, ResourceGroup> groups;
		std::map<ResourceID, ResourceEntry> resources;

		GroupID unnamedGroup = kNoGroup;

		// 選択モード
		bool isSelecting = false;
		SlotRequest selectionRequest;
		std::function<void(const ResourceRef&)> onSelected;

		// GUI
		enum class ListMode : int { Texture, Buffer };
		ListMode listMode = ListMode::Buffer;
		ResourceRef focused;	// Valueウィンドウに表示する対象
	};

	RegistryState& State() {
		static RegistryState state;
		return state;
	}

	uint32_t AlignedCBVSize(const GPUBuffer* buffer) {
		return uint32_t((buffer->GetSizeInBytes() + 255) & ~size_t(255));
	}

	std::string BufferTypeToString(uint8_t type) {
		std::string str;
		auto append = [&](BufferType t, const char* name) {
			if (type & t) {
				if (!str.empty()) str += " | ";
				str += name;
			}
			};
		append(BufferType::CBV, "CBV");
		append(BufferType::SRV, "SRV");
		append(BufferType::UAV, "UAV");
		append(BufferType::ReadBack, "ReadBack");
		return str.empty() ? "-" : str;
	}

	const char* SlotTypeToString(SlotRequest::Type type) {
		switch (type) {
		case SlotRequest::Type::CBV: return "CBV";
		case SlotRequest::Type::SRV: return "SRV";
		case SlotRequest::Type::UAV: return "UAV";
		case SlotRequest::Type::Texture: return "Texture";
		}
		return "?";
	}

	std::string GroupPath(GroupID group) {
		auto& state = State();
		std::string path;
		int guard = 0;
		while (group != kNoGroup && guard++ < 64) {
			auto it = state.groups.find(group);
			if (it == state.groups.end()) break;
			path = path.empty() ? it->second.name : it->second.name + "/" + path;
			group = it->second.parent;
		}
		return path;
	}

	std::string TextureLabel(int handle, const TextureData* texture) {
		if (!texture->GetName().empty()) {
			return texture->GetName();
		}
		return "Texture " + std::to_string(handle);
	}

}

// ============================================================
// SlotRequest
// ============================================================

SlotRequest SlotRequest::FromBuffer(BufferType bufferType, const ShaderBufferBinding& binding) {
	SlotRequest request;
	char prefix = 'b';
	switch (bufferType) {
	case BufferType::SRV:
		request.type = Type::SRV;
		prefix = 't';
		break;
	case BufferType::UAV:
		request.type = Type::UAV;
		prefix = 'u';
		break;
	default:
		request.type = Type::CBV;
		break;
	}
	request.size = binding.size;
	request.label = std::format("{}{} {}", prefix, binding.registerIndex, binding.name);
	return request;
}

SlotRequest SlotRequest::FromTexture(const ShaderTextureBinding& binding) {
	SlotRequest request;
	request.type = Type::Texture;
	request.writable = binding.isUAV;
	request.label = std::format("{}{} {}", binding.isUAV ? 'u' : 't', binding.registerIndex, binding.name);
	return request;
}

// ============================================================
// 初期化
// ============================================================

void ResourceRegistry::SetTextureManager(TextureManager* textureManager) {
	State().textureManager = textureManager;
}

// ============================================================
// グループ
// ============================================================

GroupID ResourceRegistry::CreateGroup(const std::string& name, GroupID parent, bool isVirtual) {
	auto& state = State();
	ResourceGroup group;
	group.id = state.nextGroupID++;
	group.name = name;
	group.parent = parent;
	group.isVirtual = isVirtual;
	state.groups[group.id] = group;
	return group.id;
}

void ResourceRegistry::DestroyGroup(GroupID group) {
	auto& state = State();
	state.groups.erase(group);
	// 子グループは親を失うので最上位に移す(通常は子コンテナが先に破棄される)
	for (auto& [id, g] : state.groups) {
		if (g.parent == group) {
			g.parent = kNoGroup;
		}
	}
}

void ResourceRegistry::RenameGroup(GroupID group, const std::string& name) {
	auto& state = State();
	auto it = state.groups.find(group);
	if (it != state.groups.end()) {
		it->second.name = name;
	}
}

void ResourceRegistry::SetGroupParent(GroupID group, GroupID parent) {
	auto& state = State();
	auto it = state.groups.find(group);
	if (it != state.groups.end() && group != parent) {
		it->second.parent = parent;
	}
}

GroupID ResourceRegistry::GetUnnamedGroup() {
	auto& state = State();
	if (state.unnamedGroup == kNoGroup) {
		state.unnamedGroup = CreateGroup("Unnamed", kNoGroup, true);
	}
	return state.unnamedGroup;
}

GroupID ResourceRegistry::FindOrCreateVirtualGroup(const std::string& name) {
	auto& state = State();
	for (const auto& [id, group] : state.groups) {
		if (group.isVirtual && group.parent == kNoGroup && group.name == name) {
			return id;
		}
	}
	return CreateGroup(name, kNoGroup, true);
}

// ============================================================
// リソース
// ============================================================

ResourceID ResourceRegistry::AddResource(GroupID group, GPUBuffer* buffer, const std::string& name, bool hidden, bool external) {
	auto& state = State();
	ResourceEntry entry;
	entry.id = state.nextResourceID++;
	entry.name = name.empty() ? "Buffer " + std::to_string(entry.id) : name;
	entry.group = group;
	entry.buffer = buffer;
	entry.hidden = hidden;
	entry.external = external;
	state.resources[entry.id] = entry;
	return entry.id;
}

void ResourceRegistry::RemoveResource(ResourceID id) {
	auto& state = State();
	state.resources.erase(id);
	if (state.focused == ResourceRef::Buffer(id)) {
		state.focused = {};
	}
}

void ResourceRegistry::MoveResource(ResourceID id, GroupID group) {
	auto& state = State();
	auto it = state.resources.find(id);
	if (it != state.resources.end()) {
		it->second.group = group;
	}
}

void ResourceRegistry::RenameResource(ResourceID id, const std::string& name) {
	auto& state = State();
	auto it = state.resources.find(id);
	if (it != state.resources.end() && !name.empty()) {
		it->second.name = name;
	}
}

void ResourceRegistry::SetResourceHidden(ResourceID id, bool hidden) {
	auto& state = State();
	auto it = state.resources.find(id);
	if (it != state.resources.end()) {
		it->second.hidden = hidden;
	}
}

ResourceID ResourceRegistry::RegisterExternal(const std::string& groupName, const std::string& name, GPUBuffer* buffer) {
	return AddResource(FindOrCreateVirtualGroup(groupName), buffer, name, false, true);
}

// ============================================================
// 参照
// ============================================================

const ResourceEntry* ResourceRegistry::GetEntry(ResourceID id) {
	auto& state = State();
	auto it = state.resources.find(id);
	return it != state.resources.end() ? &it->second : nullptr;
}

const ResourceGroup* ResourceRegistry::GetGroup(GroupID group) {
	auto& state = State();
	auto it = state.groups.find(group);
	return it != state.groups.end() ? &it->second : nullptr;
}

GPUBuffer* ResourceRegistry::Resolve(const ResourceRef& ref) {
	auto& state = State();
	switch (ref.source) {
	case ResourceRef::Source::Buffer:
	{
		const ResourceEntry* entry = GetEntry(ResourceID(ref.id));
		return entry ? entry->buffer : nullptr;
	}
	case ResourceRef::Source::Texture:
	{
		if (!state.textureManager) return nullptr;
		const auto& list = state.textureManager->GetTextureDataList();
		auto it = list.find(ref.id);
		return it != list.end() ? it->second.get() : nullptr;
	}
	default:
		return nullptr;
	}
}

std::string ResourceRegistry::GetDisplayName(const ResourceRef& ref) {
	switch (ref.source) {
	case ResourceRef::Source::Buffer:
	{
		const ResourceEntry* entry = GetEntry(ResourceID(ref.id));
		if (!entry) return "(削除済み)";
		std::string path = GroupPath(entry->group);
		return path.empty() ? entry->name : path + "/" + entry->name;
	}
	case ResourceRef::Source::Texture:
	{
		auto* texture = dynamic_cast<TextureData*>(Resolve(ref));
		if (!texture) return "(削除済み)";
		return TextureLabel(ref.id, texture);
	}
	default:
		return "(未割り当て)";
	}
}

bool ResourceRegistry::CanBind(const ResourceRef& ref, const SlotRequest& request) {
	GPUBuffer* buffer = Resolve(ref);
	if (!buffer) {
		return false;
	}

	const uint8_t type = buffer->GetBufferType();
	const bool isTexture = buffer->IsTexture();

	switch (request.type) {
	case SlotRequest::Type::CBV:
		// CBVは256byte単位で確保されているので、確保サイズで比較する
		return !isTexture && (type & BufferType::CBV) && AlignedCBVSize(buffer) >= request.size;
	case SlotRequest::Type::SRV:
		return !isTexture && (type & BufferType::SRV) &&
			(request.size == 0 || buffer->GetStrideInBytes() == request.size);
	case SlotRequest::Type::UAV:
		return !isTexture && (type & BufferType::UAV) &&
			(request.size == 0 || buffer->GetStrideInBytes() == request.size);
	case SlotRequest::Type::Texture:
		return isTexture && (!request.writable || (type & BufferType::UAV));
	}
	return false;
}

// ============================================================
// 選択
// ============================================================

void ResourceRegistry::BeginSelection(const SlotRequest& request, std::function<void(const ResourceRef&)> onSelected) {
	auto& state = State();
	state.isSelecting = true;
	state.selectionRequest = request;
	state.onSelected = std::move(onSelected);
	// 要求に合わせて一覧の種類を切り替える
	state.listMode = request.type == SlotRequest::Type::Texture ? RegistryState::ListMode::Texture : RegistryState::ListMode::Buffer;
}

void ResourceRegistry::CancelSelection() {
	auto& state = State();
	state.isSelecting = false;
	state.onSelected = nullptr;
}

bool ResourceRegistry::IsSelecting() {
	return State().isSelecting;
}

// ============================================================
// ImGui
// ============================================================

#ifdef USE_IMGUI
namespace {

	// 一覧の1項目。クリックでフォーカス、選択モード中なら割り当てて終了
	void DrawItem(const ResourceRef& ref, const std::string& label) {
		auto& state = State();
		const bool selectable = !state.isSelecting || ResourceRegistry::CanBind(ref, state.selectionRequest);
		const bool focused = state.focused == ref;

		ImGui::PushID(int(ref.source));
		ImGui::PushID(ref.id);
		if (!selectable) {
			ImGui::BeginDisabled();
		}
		std::string text = (focused ? "> " : "  ") + label;
		if (ImGui::Selectable(text.c_str(), focused)) {
			state.focused = ref;
			if (state.isSelecting && state.onSelected) {
				auto callback = std::move(state.onSelected);
				ResourceRegistry::CancelSelection();
				callback(ref);
			}
		}
		if (!selectable) {
			ImGui::EndDisabled();
		}
		ImGui::PopID();
		ImGui::PopID();
	}

	// グループ内に表示する項目があるか(子グループも含む)
	bool HasVisibleItems(GroupID group) {
		auto& state = State();
		for (const auto& [id, entry] : state.resources) {
			if (entry.group == group && !entry.hidden) return true;
		}
		for (const auto& [id, g] : state.groups) {
			if (g.parent == group && HasVisibleItems(id)) return true;
		}
		return false;
	}

	void DrawGroupTree(GroupID group) {
		auto& state = State();
		const ResourceGroup& info = state.groups.at(group);
		if (!HasVisibleItems(group)) {
			return;
		}

		ImGui::PushID(int(group));
		if (ImGui::TreeNodeEx(info.name.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
			// 子グループ
			for (const auto& [id, g] : state.groups) {
				if (g.parent == group) {
					DrawGroupTree(id);
				}
			}
			// 項目
			for (const auto& [id, entry] : state.resources) {
				if (entry.group == group && !entry.hidden) {
					DrawItem(ResourceRef::Buffer(id), entry.name);
				}
			}
			ImGui::TreePop();
		}
		ImGui::PopID();
	}

	// テクスチャ一覧のフォルダ(ツリーの1階層)
	struct TextureFolder {
		std::map<std::string, TextureFolder> folders;
		std::vector<std::pair<int, TextureData*>> items;
	};

	bool HasTextureItems(const TextureFolder& folder) {
		if (!folder.items.empty()) return true;
		for (const auto& [name, child] : folder.folders) {
			if (HasTextureItems(child)) return true;
		}
		return false;
	}

	void DrawTextureFolder(const std::string& name, const TextureFolder& folder, bool defaultOpen) {
		if (!HasTextureItems(folder)) {
			return;
		}
		if (ImGui::TreeNodeEx(name.c_str(), defaultOpen ? ImGuiTreeNodeFlags_DefaultOpen : ImGuiTreeNodeFlags_None)) {
			for (const auto& [childName, child] : folder.folders) {
				DrawTextureFolder(childName, child, false);
			}
			for (const auto& [handle, texture] : folder.items) {
				DrawItem(ResourceRef::Texture(handle), TextureLabel(handle, texture));
			}
			ImGui::TreePop();
		}
	}

	// RTV / DSV / Load(実際のフォルダ階層) / Other に振り分けて表示する
	void DrawTextureTree() {
		auto& state = State();
		if (!state.textureManager) {
			return;
		}

		TextureFolder rtv, dsv, load, other;
		for (const auto& [handle, texture] : state.textureManager->GetTextureDataList()) {
			TextureData* data = texture.get();
			if (data->HasRTV()) {
				rtv.items.emplace_back(handle, data);
			} else if (data->HasDSV()) {
				dsv.items.emplace_back(handle, data);
			} else if (!data->GetFilePath().empty()) {
				const std::string& path = data->GetFilePath();
				// エンジン内部用のテクスチャは表示しない
				if (path.starts_with(".EngineResource")) {
					continue;
				}
				// "Texture/UI/Icon.png" → Texture > UI > Icon.png
				TextureFolder* current = &load;
				size_t begin = 0;
				size_t slash = path.find('/');
				while (slash != std::string::npos) {
					current = &current->folders[path.substr(begin, slash - begin)];
					begin = slash + 1;
					slash = path.find('/', begin);
				}
				current->items.emplace_back(handle, data);
			} else {
				// フォントなど、ファイルから読み込んでいないテクスチャ
				other.items.emplace_back(handle, data);
			}
		}

		DrawTextureFolder("RTV", rtv, true);
		DrawTextureFolder("DSV", dsv, true);
		DrawTextureFolder("Load", load, true);
		DrawTextureFolder("Other", other, false);
	}

	void DrawResourceWindow() {
		auto& state = State();

		ImGui::Begin("Resource##ResourceRegistry");

		// ウィンドウの種類(一番上の行)
		int mode = int(state.listMode);
		ImGui::RadioButton("Texture", &mode, int(RegistryState::ListMode::Texture));
		ImGui::SameLine();
		ImGui::RadioButton("Buffer", &mode, int(RegistryState::ListMode::Buffer));
		state.listMode = RegistryState::ListMode(mode);

		if (state.isSelecting) {
			ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "割り当て先: %s [%s]",
				state.selectionRequest.label.c_str(), SlotTypeToString(state.selectionRequest.type));
			ImGui::SameLine();
			if (ImGui::SmallButton("キャンセル")) {
				ResourceRegistry::CancelSelection();
			}
		}
		ImGui::Separator();

		ImGui::BeginChild("ResourceList");
		if (state.listMode == RegistryState::ListMode::Texture) {
			DrawTextureTree();
		} else {
			// 最上位のグループから(Unnamedは最後)
			for (const auto& [id, group] : state.groups) {
				if (group.parent == kNoGroup && id != state.unnamedGroup) {
					DrawGroupTree(id);
				}
			}
			if (state.unnamedGroup != kNoGroup) {
				DrawGroupTree(state.unnamedGroup);
			}
		}
		ImGui::EndChild();

		ImGui::End();
	}

	void PropertyRow(const char* name, const std::string& value) {
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::TextUnformatted(name);
		ImGui::TableSetColumnIndex(1);
		ImGui::TextUnformatted(value.c_str());
	}

	void DrawTextureValue(int handle, TextureData* texture) {
		auto [width, height] = texture->GetSize();

		if (ImGui::BeginTable("TextureHeader", 2, ImGuiTableFlags_BordersInner)) {
			PropertyRow(TextureLabel(handle, texture).c_str(), std::format("{}x{}", width, height));
			ImGui::EndTable();
		}

		// プレビュー。描画先として使用中(シェーダーから読めない状態)のテクスチャやCubeは表示しない
		const D3D12_RESOURCE_STATES state = texture->GetCurrentState();
		const bool readable = (state & D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE) != 0;
		const bool previewable = readable && texture->GetType() == TextureData::Type::Normal && width > 0 && height > 0;
		if (previewable) {
			float availableWidth = ImGui::GetContentRegionAvail().x;
			float scale = std::min(availableWidth / float(width), 256.0f / float(height));
			ImGui::Image(ImTextureRef(texture->GetSRVHandle().ptr), ImVec2(width * scale, height * scale));
		} else {
			ImGui::TextDisabled("(プレビュー不可: 描画中またはCubeテクスチャ)");
		}

		if (ImGui::BeginTable("TextureProperty", 2, ImGuiTableFlags_BordersInner | ImGuiTableFlags_RowBg)) {
			PropertyRow("RTV", texture->HasRTV() ? "TRUE" : "FALSE");
			PropertyRow("DSV", texture->HasDSV() ? "TRUE" : "FALSE");
			PropertyRow("UAV", texture->IsUnordered() ? "TRUE" : "FALSE");
			PropertyRow("SRVインデックス", std::to_string(texture->GetHandle()));
			PropertyRow("Format", std::to_string(int(texture->GetFormat())));
			PropertyRow("Full Path", texture->GetFilePath().empty() ? "-" : "Assets/" + texture->GetFilePath());
			ImGui::EndTable();
		}
	}

	void DrawBufferValue(const ResourceEntry& entry) {
		GPUBuffer* buffer = entry.buffer;

		if (ImGui::BeginTable("BufferHeader", 2, ImGuiTableFlags_BordersInner)) {
			PropertyRow(entry.name.c_str(), std::format("{} byte", buffer->GetSizeInBytes()));
			ImGui::EndTable();
		}

		if (ImGui::BeginTable("BufferProperty", 2, ImGuiTableFlags_BordersInner | ImGuiTableFlags_RowBg)) {
			PropertyRow("Group", GroupPath(entry.group));
			PropertyRow("Type", BufferTypeToString(buffer->GetBufferType()));
			PropertyRow("Stride", std::format("{} byte", buffer->GetStrideInBytes()));
			PropertyRow("Count", std::to_string(buffer->GetNum()));
			PropertyRow("Size", std::format("{} byte", buffer->GetSizeInBytes()));
			PropertyRow("External", entry.external ? "TRUE" : "FALSE");
			ImGui::EndTable();
		}
	}

	void DrawValueWindow() {
		auto& state = State();

		ImGui::Begin("Value##ResourceRegistry");

		GPUBuffer* buffer = ResourceRegistry::Resolve(state.focused);
		if (!buffer) {
			ImGui::TextDisabled("リソースを選択してください");
		} else if (state.focused.source == ResourceRef::Source::Texture) {
			if (auto* texture = dynamic_cast<TextureData*>(buffer)) {
				DrawTextureValue(state.focused.id, texture);
			}
		} else if (const ResourceEntry* entry = ResourceRegistry::GetEntry(ResourceID(state.focused.id))) {
			DrawBufferValue(*entry);
		}

		ImGui::End();
	}

}
#endif

void ResourceRegistry::DrawImGui() {
#ifdef USE_IMGUI
	DrawResourceWindow();
	DrawValueWindow();
#endif
}
