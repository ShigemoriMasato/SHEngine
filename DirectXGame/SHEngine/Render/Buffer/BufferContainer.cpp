#include "BufferContainer.h"

SHEngine::BufferContainer::BufferContainer(uint32_t size, const std::string& name) {
	buffers_.reserve(size);
	if (!name.empty()) {
		SetName(name);
	}
}

SHEngine::BufferContainer::~BufferContainer() {
	// 子コンテナを先に破棄する(子のグループが親グループを参照しているため)
	children_.clear();

	for (auto& entry : buffers_) {
		ResourceRegistry::RemoveResource(entry.id);
	}
	if (group_ != kNoGroup) {
		ResourceRegistry::DestroyGroup(group_);
	}

	// GPUBufferの破棄時に、リソースとディスクリプタはDeferredReleaseQueueで数フレーム待ってから解放される
}

SHEngine::GPUBuffer* SHEngine::BufferContainer::Create(BufferType bufferType, size_t byte, uint32_t num, BufferNum bufferNum, const std::string& name) {
	auto& entry = buffers_.emplace_back();
	entry.buffer = std::make_unique<GPUBuffer>(bufferType, byte, num, bufferNum);
	entry.id = ResourceRegistry::AddResource(GetTargetGroup(), entry.buffer.get(), name, hidden_);
	// ID3D12Resourceにも一覧と同じ名前("グループ/名前")を付ける
	entry.buffer->SetName(ResourceRegistry::GetDisplayName(ResourceRef::Buffer(entry.id)));
	return entry.buffer.get();
}

void SHEngine::BufferContainer::ApplyResourceNames() {
	for (auto& entry : buffers_) {
		entry.buffer->SetName(ResourceRegistry::GetDisplayName(ResourceRef::Buffer(entry.id)));
	}
	for (auto& child : children_) {
		child->ApplyResourceNames();
	}
}

void SHEngine::BufferContainer::Erase(GPUBuffer*& buffer) {
	for (auto it = buffers_.begin(); it != buffers_.end(); ++it) {
		if (it->buffer.get() == buffer) {
			ResourceRegistry::RemoveResource(it->id);
			// GPUがまだ使用している可能性があるが、GPUBufferの破棄時にDeferredReleaseQueueへ預けられるのでそのまま消してよい
			buffers_.erase(it);
			buffer = nullptr;
			return;
		}
	}
	assert(false && "BufferContainer::Erase: buffer not found");
}

SHEngine::BufferContainer* SHEngine::BufferContainer::CreateChild(const std::string& name) {
	auto& child = children_.emplace_back(std::make_unique<BufferContainer>(32, name.empty() ? "Child" : name));
	child->parent_ = this;
	child->hidden_ = hidden_;
	ResourceRegistry::SetGroupParent(child->group_, GetParentGroupForChild());
	child->ApplyResourceNames();
	return child.get();
}

void SHEngine::BufferContainer::SetName(const std::string& name) {
	name_ = name;

	if (group_ != kNoGroup) {
		// 既に自分のグループがある場合は名前を変えるだけ
		ResourceRegistry::RenameGroup(group_, name_.empty() ? "Unnamed" : name_);
	} else if (!name_.empty()) {
		// Unnamedから自分のグループへ移る
		group_ = ResourceRegistry::CreateGroup(name_, parent_ ? parent_->GetParentGroupForChild() : kNoGroup);
		for (auto& entry : buffers_) {
			ResourceRegistry::MoveResource(entry.id, group_);
		}
		// 子グループの親も付け替える
		for (auto& child : children_) {
			ResourceRegistry::SetGroupParent(child->group_, group_);
		}
	}

	// グループのパスが変わったので、ID3D12Resourceの名前も付け直す
	ApplyResourceNames();
}

void SHEngine::BufferContainer::SetHidden(bool hidden) {
	hidden_ = hidden;
	for (auto& entry : buffers_) {
		ResourceRegistry::SetResourceHidden(entry.id, hidden_);
	}
}

SHEngine::ResourceID SHEngine::BufferContainer::GetResourceID(const GPUBuffer* buffer) const {
	for (const auto& entry : buffers_) {
		if (entry.buffer.get() == buffer) {
			return entry.id;
		}
	}
	return kInvalidResourceID;
}

SHEngine::GroupID SHEngine::BufferContainer::GetTargetGroup() const {
	return group_ != kNoGroup ? group_ : ResourceRegistry::GetUnnamedGroup();
}

