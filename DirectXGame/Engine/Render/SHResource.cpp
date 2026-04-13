#include "SHResource.h"
#include <Utility/DirectUtilFuncs.h>
#include <DirectXTex/d3dx12.h>
#define Sigma(x) for (uint32_t i = 0; i < x; ++i)

SHEngine::GPUBuffer::GPUBuffer(ResourceDesc& desc) {
	//この組み合わせしたら殺す
	assert(!((desc.bufferType & BufferType::CBV) && (desc.bufferType & BufferType::UAV)));

	sizeInBytes_ = desc.sizeInBytes * desc.elementCount;
	UINT size = 0;
	if (desc.bufferType & uint8_t(BufferType::CBV)) {
		size = (static_cast<UINT>(sizeInBytes_) + 255) & ~255;	//CBVは256バイトアラインメント
	} else {
		size = static_cast<UINT>(sizeInBytes_);
	}

	resources_.reserve(desc.bufferNum);
	mappedData_.reserve(desc.bufferNum);
	currentState_.resize(desc.bufferNum, desc.initialState);
	auto heapType = ((desc.bufferType & BufferType::UAV) == 0U ? D3D12_HEAP_TYPE_UPLOAD : D3D12_HEAP_TYPE_DEFAULT);

	for (uint32_t i = 0; i < desc.bufferNum; ++i) {
		//頂点リソース用のヒープの設定
		D3D12_HEAP_PROPERTIES uploadHeapProperties{};
		uploadHeapProperties.Type = ((desc.bufferType & BufferType::UAV) == 0U ? D3D12_HEAP_TYPE_UPLOAD : D3D12_HEAP_TYPE_DEFAULT);
		//頂点リソースの設定
		D3D12_RESOURCE_DESC bufferResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(size);
		//バッファリソース、テクスチャの場合はまた別の設定をする
		bufferResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		bufferResourceDesc.Width = UINT64(size);
		bufferResourceDesc.Height = 1;
		//バッファの場合はこれにする決まり
		bufferResourceDesc.DepthOrArraySize = 1;
		bufferResourceDesc.MipLevels = 1;
		bufferResourceDesc.SampleDesc.Count = 1;
		//バッファの場合はこれにする決まり
		bufferResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

		auto& bufferResource = resources_.emplace_back().res;

		HRESULT reason = device_->GetDevice()->GetDeviceRemovedReason();

		HRESULT hr = device_->GetDevice()->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE,
			&bufferResourceDesc, desc.initialState, nullptr,
			IID_PPV_ARGS(&bufferResource));
		assert(SUCCEEDED(hr));

		if ((desc.bufferType & BufferType::UAV) == 0) {
			auto& mapped = mappedData_.emplace_back();
			hr = bufferResource->Map(0, nullptr, &mapped);
		}
	}

	auto HasBuffer = [&](BufferType t) noexcept -> bool {
		return (desc.bufferType & static_cast<uint8_t>(t)) != 0u;
		};

	assert(desc.bufferType & 0b111);	//SRV/CBV/UAVのどれかが指定されていること

	if (HasBuffer(BufferType::CBV)) {
		for (uint32_t i = 0; i < desc.bufferNum; ++i) {
			descriptorHandles_[BufferType::CBV].push_back(static_cast<D3D12_GPU_DESCRIPTOR_HANDLE>(resources_[i].res->GetGPUVirtualAddress()));
		}
	}

	if (HasBuffer(BufferType::SRV)) {
		for (uint32_t i = 0; i < desc.bufferNum; ++i) {
			auto& res = resources_[i].res;

			// SRVハンドルの取得
			std::unique_ptr<SRVHandle>& srvHandle = srvHandles_.emplace_back();
			srvHandle = std::make_unique<SRVHandle>();
			srvHandle->UpdateHandle(device_->GetSRVManager());

			// ParticleDataのSRV作成
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
			srvDesc.Format = DXGI_FORMAT_UNKNOWN;
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
			srvDesc.Buffer.FirstElement = 0;
			srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
			srvDesc.Buffer.NumElements = desc.elementCount;
			srvDesc.Buffer.StructureByteStride = UINT(desc.sizeInBytes);

			device_->GetDevice()->CreateShaderResourceView(res.Get(), &srvDesc, srvHandle->GetCPU());

			descriptorHandles_[BufferType::SRV].push_back(srvHandle->GetGPU());
		}
	}

	if (HasBuffer(BufferType::UAV)) {
		for (uint32_t i = 0; i < desc.bufferNum; ++i) {
			auto& res = resources_[i].res;

			// UAVハンドルの取得
			std::unique_ptr<SRVHandle>& uavHandle = uavHandles_.emplace_back();
			uavHandle = std::make_unique<SRVHandle>();
			uavHandle->UpdateHandle(device_->GetSRVManager());

			// ParticleDataのUAV作成
			D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
			uavDesc.Format = DXGI_FORMAT_UNKNOWN;
			uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
			uavDesc.Buffer.FirstElement = 0;
			uavDesc.Buffer.NumElements = desc.elementCount;
			uavDesc.Buffer.CounterOffsetInBytes = 0;
			uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
			uavDesc.Buffer.StructureByteStride = UINT(desc.sizeInBytes);

			device_->GetDevice()->CreateUnorderedAccessView(res.Get(), nullptr, &uavDesc, uavHandle->GetCPU());

			descriptorHandles_[BufferType::UAV].push_back(uavHandle->GetGPU());
		}
	}
}

D3D12_GPU_DESCRIPTOR_HANDLE SHEngine::GPUBuffer::GetGPUDescriptorHandle(BufferType type, uint32_t bufferIndex) const {
	auto it = descriptorHandles_.find(type);
	assert(it != descriptorHandles_.end());
	return it->second.at(bufferIndex);
}

void SHEngine::GPUBuffer::CopyBuffer(const void* data, size_t dataSize) {
	//安全性担保
	assert(dataSize <= sizeInBytes_);
	if (!uavHandles_.empty()) {
		assert(false && "Cant copy data to UAV buffer");
		return;
	}

	// 所有を持つバイト配列にコピーして保持する（呼び出し元の寿命に依存しない）
	nextData_.assign(reinterpret_cast<const uint8_t*>(data), reinterpret_cast<const uint8_t*>(data) + dataSize);
}

void SHEngine::GPUBuffer::TransitionBarrier(D3D12_RESOURCE_STATES after) {
	nextState_ = after;
}

void SHEngine::GPUBuffer::Flush(CmdObj* cmdObj, uint32_t bufferIndex) {
	if (uavHandles_.empty()) {
		std::memcpy(mappedData_[bufferIndex], nextData_.data(), nextData_.size());
	}

	//同じだった場合の処理と過去の状態の更新は関数内に含まれている
	Func::InsertBarrier(cmdObj->GetCommandList(), currentState_[bufferIndex], nextState_, resources_[bufferIndex].res.Get());
}

BufferType operator|(BufferType a, BufferType b) {
	return static_cast<BufferType>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}

BufferType operator|(uint8_t a, BufferType b) {
	return static_cast<BufferType>(a | static_cast<uint8_t>(b));
}

uint8_t operator&(uint8_t a, BufferType b) {
	return (a & static_cast<uint8_t>(b));
}

uint8_t operator~(BufferType a) {
	return ~static_cast<uint8_t>(a);
}
