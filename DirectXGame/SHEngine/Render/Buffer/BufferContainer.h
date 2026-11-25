#pragma once
#include "GPUBuffer.h"
#include "ResourceRegistry.h"

namespace SHEngine {

	// @brief GPUBuffer用のクラス。作成と寿命管理を行う。GPUBufferの作成関数やemplace_back等で1行が長くなるので、その解決用として存在している。
	// @brief ただ、GPUBufferの数が多くなると管理が大変なので、CBVだけでも生ポインタでいいからメンバ変数に持っておくことをおすすめする。
	// @note 作成したGPUBufferはResourceRegistryに登録され、ImGuiの一覧やノードへの割り当てに使用される。
	//       1つのBufferContainerが一覧上の1グループになる。名前を付けない場合は "Unnamed" グループに入る。
	class BufferContainer {
	public:

		// @param size 予約するバッファ数
		// @param name グループ名。空の場合は "Unnamed" グループに入る
		BufferContainer(uint32_t size = 32, const std::string& name = "");
		~BufferContainer();

		BufferContainer(const BufferContainer&) = delete;
		BufferContainer& operator=(const BufferContainer&) = delete;

		// @brief GPUBufferの作成。作られた順番に0からインデックスが振られる。Copyするときはこのインデックスを指定する。
		// @param bufferType バッファの種類（CBV、SRV、UAVの組み合わせ）
		// @param byte バッファのサイズ（バイト単位）
		// @param num バッファの数（デフォルトは1）
		// @param bufferNum バッファの数（デフォルトは3、スワップチェーンのバッファ数に合わせる）
		// @param name 一覧に表示する名前。空の場合は "Buffer <ID>" になる
		GPUBuffer* Create(BufferType bufferType, size_t byte, uint32_t num = 1, BufferNum bufferNum = BufferNum::MatchSwapChain, const std::string& name = "");

		// @brief GPUBufferへのデータコピー。indexはCreateしたときの順番。UAVバッファにはコピーできない。
		// @param index GPUBufferのインデックス。Createされた順番で0から振られる。
		// @param data コピーするデータのポインタ。
		// @param dataSize コピーするデータのサイズ（バイト単位）
		void Copy(uint32_t index, const void* data, size_t dataSize) {
			assert(index < buffers_.size());
			buffers_[index].buffer->CopyBuffer(data, dataSize);
		}

		// @brief GPUBufferを解放する。GPU上のリソースはDeferredReleaseQueueで数フレーム待機したのち解放される。生ポインタは無効になるので注意。
		void Erase(GPUBuffer*& buffer);

		// @brief 一覧上のグループ分け用に子コンテナを作成する。子コンテナはこのコンテナが所有し、一緒に破棄される。
		// @param name 子グループ名
		BufferContainer* CreateChild(const std::string& name);

		// @brief グループ名を設定する。"Unnamed" に入っていたバッファは自分のグループに移る
		void SetName(const std::string& name);
		const std::string& GetName() const { return name_; }

		// @brief 一覧に表示しないようにする(Rendererの頂点バッファなど、ノードから使わないもの)。作成済み・今後作成するバッファ両方に適用される
		void SetHidden(bool hidden);

		// @brief 作成したGPUBufferのResourceRegistry上のID。見つからない場合はkInvalidResourceID
		ResourceID GetResourceID(const GPUBuffer* buffer) const;

	private:

		// @brief バッファを登録するグループ(名前がなければUnnamed)
		GroupID GetTargetGroup() const;
		// @brief 子コンテナのグループの親にするグループ
		GroupID GetParentGroupForChild() const { return GetTargetGroup(); }
		// @brief 作成済みバッファ(子コンテナも含む)のID3D12Resourceに "グループ/名前" を付け直す
		void ApplyResourceNames();

		struct Entry {
			std::unique_ptr<GPUBuffer> buffer;
			ResourceID id = kInvalidResourceID;
		};
		std::vector<Entry> buffers_;

		std::vector<std::unique_ptr<BufferContainer>> children_;
		BufferContainer* parent_ = nullptr;

		std::string name_;
		// 自分専用のグループ(名前を付けたときに作られる)。なければUnnamedに入る
		GroupID group_ = kNoGroup;
		bool hidden_ = false;

	};

}
