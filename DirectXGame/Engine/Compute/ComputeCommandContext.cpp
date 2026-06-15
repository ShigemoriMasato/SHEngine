#include "ComputeCommandContext.h"

void SHEngine::ComputeCommandContext::Initialize(DXDevice* device, int initCmdObjNum) {
	PrivateInitialize(device, Command::Type::Compute, initCmdObjNum);

	D3D12_QUERY_HEAP_DESC desc{};
	desc.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
	desc.Count = 256;

	device->GetDevice()->CreateQueryHeap(&desc, IID_PPV_ARGS(&queryHeap_));
}
