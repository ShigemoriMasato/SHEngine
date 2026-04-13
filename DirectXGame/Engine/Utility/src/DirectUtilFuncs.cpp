#include "../DirectUtilFuncs.h"
#include <DirectXTex/d3dx12.h>
#include <dxgidebug.h>
#include <dxgi1_3.h>

#include <cassert>

using namespace SHEngine;

D3DResourceLeakChecker::~D3DResourceLeakChecker() {
	Microsoft::WRL::ComPtr<IDXGIDebug1> debug;
	if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debug)))) {
		debug->ReportLiveObjects(DXGI_DEBUG_D3D12, DXGI_DEBUG_RLO_ALL);
		debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
		debug->ReportLiveObjects(DXGI_DEBUG_APP, DXGI_DEBUG_RLO_ALL);
	}
}

ID3D12Resource* SHEngine::Func::CreateBufferResource(ID3D12Device* device, size_t size) {
	//頂点リソース用のヒープの設定
	D3D12_HEAP_PROPERTIES uploadHeapProperties{};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;//uploadHeapを使う
	//頂点リソースの設定
	D3D12_RESOURCE_DESC bufferResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(size);
	//バッファリソース、テクスチャの場合はまた別の設定をする
	bufferResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	bufferResourceDesc.Width = size;
	//バッファの場合はこれにする決まり
	bufferResourceDesc.Height = 1;
	bufferResourceDesc.DepthOrArraySize = 1;
	bufferResourceDesc.MipLevels = 1;
	bufferResourceDesc.SampleDesc.Count = 1;
	//バッファの場合はこれにする決まり
	bufferResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	ID3D12Resource* bufferResource = nullptr;

	HRESULT reason = device->GetDeviceRemovedReason();

	HRESULT hr = device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE,
		&bufferResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
		IID_PPV_ARGS(&bufferResource));
	assert(SUCCEEDED(hr));

	return bufferResource;
}

ID3D12Resource* SHEngine::Func::CreateUAVResource(ID3D12Device* device, size_t bufferSize) {
	D3D12_HEAP_PROPERTIES uploadHeapProperties{};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	//頂点リソースの設定
	D3D12_RESOURCE_DESC bufferResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);
	//バッファリソース、テクスチャの場合はまた別の設定をする
	bufferResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	bufferResourceDesc.Width = bufferSize;
	//バッファの場合はこれにする決まり
	bufferResourceDesc.Height = 1;
	bufferResourceDesc.DepthOrArraySize = 1;
	bufferResourceDesc.MipLevels = 1;
	bufferResourceDesc.SampleDesc.Count = 1;
	//バッファの場合はこれにする決まり
	bufferResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	bufferResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	ID3D12Resource* bufferResource = nullptr;

	HRESULT reason = device->GetDeviceRemovedReason();

	HRESULT hr = device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE,
		&bufferResourceDesc, D3D12_RESOURCE_STATE_COMMON, nullptr,
		IID_PPV_ARGS(&bufferResource));
	assert(SUCCEEDED(hr));

	return bufferResource;
}

ID3D12Resource* SHEngine::Func::CreateReadBackResource(ID3D12Device* device, size_t bufferSize) {
	// Readback用のヒープ設定（CPUから読み取り可能）
	D3D12_HEAP_PROPERTIES readbackHeapProperties{};
	readbackHeapProperties.Type = D3D12_HEAP_TYPE_READBACK;

	// バッファリソースの設定
	D3D12_RESOURCE_DESC bufferResourceDesc{};
	bufferResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	bufferResourceDesc.Width = bufferSize;
	bufferResourceDesc.Height = 1;
	bufferResourceDesc.DepthOrArraySize = 1;
	bufferResourceDesc.MipLevels = 1;
	bufferResourceDesc.SampleDesc.Count = 1;
	bufferResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	ID3D12Resource* bufferResource = nullptr;

	HRESULT hr = device->CreateCommittedResource(
		&readbackHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&bufferResourceDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&bufferResource));
	assert(SUCCEEDED(hr));

	return bufferResource;
}

void SHEngine::Func::InsertBarrier(ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_STATES stateAfter, D3D12_RESOURCE_STATES& stateBefore, ID3D12Resource* pResource,
	D3D12_RESOURCE_BARRIER_TYPE type, D3D12_RESOURCE_BARRIER_FLAGS flags) {

	if (stateAfter == stateBefore) {
		// 既に同じ状態ならバリアは不要
		return;
	}

	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = type;
	barrier.Flags = flags;
	barrier.Transition.pResource = pResource;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = stateBefore;
	barrier.Transition.StateAfter = stateAfter;

	commandList->ResourceBarrier(1, &barrier);

	// 状態を更新
	stateBefore = stateAfter;
}
