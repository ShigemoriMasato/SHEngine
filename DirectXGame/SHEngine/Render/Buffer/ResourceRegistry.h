#pragma once
#include <Render/Buffer/GPUBuffer.h>
#include <Render/PSO/Shelf/ShaderReflection.h>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace SHEngine {

	class TextureManager;

	using ResourceID = uint32_t;
	using GroupID = uint32_t;
	inline constexpr ResourceID kInvalidResourceID = 0;
	inline constexpr GroupID kNoGroup = 0;

	// @brief ノードのスロットなどが持つ、リソースへの参照
	// @note バッファはResourceRegistryのID、テクスチャはTextureManagerのハンドルで指す
	struct ResourceRef {
		enum class Source : uint8_t {
			None,
			Buffer,
			Texture,
		};
		Source source = Source::None;
		int id = -1;

		static ResourceRef Buffer(ResourceID id) { return { Source::Buffer, int(id) }; }
		static ResourceRef Texture(int textureHandle) { return { Source::Texture, textureHandle }; }

		bool IsNone() const { return source == Source::None; }
		bool operator==(const ResourceRef& other) const { return source == other.source && id == other.id; }
	};

	// @brief スロットが要求するリソースの条件
	struct SlotRequest {
		enum class Type : uint8_t {
			CBV,
			SRV,
			UAV,
			Texture,
		};
		Type type = Type::CBV;
		// CBV: 必要なサイズ(byte) / SRV・UAV: 1要素のサイズ(0なら問わない) / Texture: 未使用
		uint32_t size = 0;
		// Texture: 書き込み(RWTexture)が必要か
		bool writable = false;
		// 選択画面に表示する説明
		std::string label;

		// @brief リフレクションのバインド情報から作成する
		static SlotRequest FromBuffer(BufferType bufferType, const ShaderBufferBinding& binding);
		static SlotRequest FromTexture(const ShaderTextureBinding& binding);
	};

	// @brief リソースのグループ(BufferContainer 1つ、または外部登録用の仮想グループ)
	struct ResourceGroup {
		GroupID id = kNoGroup;
		std::string name;
		GroupID parent = kNoGroup;	// 親グループ(子コンテナの場合)
		bool isVirtual = false;		// 外部登録用など、BufferContainerを持たないグループ
	};

	// @brief 登録されたバッファ1つ分の情報
	struct ResourceEntry {
		ResourceID id = kInvalidResourceID;
		std::string name;
		GroupID group = kNoGroup;
		GPUBuffer* buffer = nullptr;	// 所有はしない
		bool hidden = false;			// 一覧に表示しない(Rendererの頂点バッファなど)
		bool external = false;			// BufferContainer以外が所有している
	};

	// @brief 全GPUBuffer(とテクスチャ)の一覧を管理し、ImGuiでの表示とスロットへの割り当てを行う。
	// @note BufferContainerが作成・削除を報告してくる。所有はしない。
	class ResourceRegistry {
	public:

		ResourceRegistry() = delete;

		// ---- 初期化 ----

		// @brief テクスチャ一覧の取得元を設定する(Engineが設定する)
		static void SetTextureManager(TextureManager* textureManager);

		// ---- グループ (BufferContainerから呼ばれる) ----

		// @brief グループを作成する
		static GroupID CreateGroup(const std::string& name, GroupID parent = kNoGroup, bool isVirtual = false);
		// @brief グループを削除する。中のリソースは先にRemoveResourceしておくこと
		static void DestroyGroup(GroupID group);
		static void RenameGroup(GroupID group, const std::string& name);
		static void SetGroupParent(GroupID group, GroupID parent);
		// @brief 名前を付けずに作ったBufferContainerのリソースが入るグループ
		static GroupID GetUnnamedGroup();
		// @brief 名前で最上位の仮想グループを探し、なければ作成する(外部登録用)
		static GroupID FindOrCreateVirtualGroup(const std::string& name);

		// ---- リソース ----

		static ResourceID AddResource(GroupID group, GPUBuffer* buffer, const std::string& name, bool hidden = false, bool external = false);
		static void RemoveResource(ResourceID id);
		static void MoveResource(ResourceID id, GroupID group);
		static void RenameResource(ResourceID id, const std::string& name);
		static void SetResourceHidden(ResourceID id, bool hidden);

		// @brief BufferContainer以外が所有するバッファを登録する(カメラなど)。所有者が破棄する前にRemoveResourceすること
		// @param groupName 登録先の仮想グループ名
		static ResourceID RegisterExternal(const std::string& groupName, const std::string& name, GPUBuffer* buffer);

		// ---- 参照 ----

		static const ResourceEntry* GetEntry(ResourceID id);
		static const ResourceGroup* GetGroup(GroupID group);
		// @brief 参照先のGPUBufferを取得する。削除済みならnullptr
		static GPUBuffer* Resolve(const ResourceRef& ref);
		// @brief 表示・保存用の名前 ("Group/Child/Name" や テクスチャ名)
		static std::string GetDisplayName(const ResourceRef& ref);
		// @brief 参照先をスロットに割り当てられるか
		static bool CanBind(const ResourceRef& ref, const SlotRequest& request);

		// ---- 選択 (ノード画面のスロット → リソース画面) ----

		// @brief 選択モードを開始する。リソース画面で割り当て可能なものをクリックするとonSelectedが呼ばれ、選択モードが終わる
		static void BeginSelection(const SlotRequest& request, std::function<void(const ResourceRef&)> onSelected);
		static void CancelSelection();
		static bool IsSelecting();

		// ---- ImGui ----

		// @brief "Resource" と "Value" ウィンドウを描画する
		static void DrawImGui();
	};

}
