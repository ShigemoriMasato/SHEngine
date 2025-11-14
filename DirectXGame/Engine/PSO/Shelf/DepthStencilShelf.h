#pragma once
#include <cstdint>
#include <vector>
#include <d3d12.h>

enum class DepthStencilID : uint8_t {
	Default,		//不透明
	Transparent,	//透明なとき(描画順に注意)

	Count
};

class DepthStencilShelf {
public:

	DepthStencilShelf();
	~DepthStencilShelf();
	
	D3D12_DEPTH_STENCIL_DESC GetDepthStencilDesc(DepthStencilID id) const;

private:

	std::vector<D3D12_DEPTH_STENCIL_DESC> depthStencilDescs_;

};

