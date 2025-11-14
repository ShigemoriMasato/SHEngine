#pragma once
#include <vector>
#include <cstdint>
#include <d3d12.h>

enum class RasterizerID : uint8_t {
	Fill,
	Wireframe,

	Count
};

class RasterizerShelf {
public:

	RasterizerShelf();
	~RasterizerShelf();

	D3D12_RASTERIZER_DESC GetRasterizerDesc(RasterizerID id) const;

private:

	std::vector<D3D12_RASTERIZER_DESC> rasterizerDescs_;

};
