#pragma once
#include <d3d12.h>
#include <DirectXTex/d3dx12.h>
#include <cassert>

ID3D12Resource* CreateBufferResource(ID3D12Device* device, size_t sizeInBytes);
