#pragma once
#include <cstdint>
#include <d3d12.h>
#include <vector>

enum class BlendStateID : uint8_t {
	Normal,
	Add,
	Subtract,
	Multiply,
	Screen,

	Count
};

class BlendStateShelf {
public:

	BlendStateShelf();
	~BlendStateShelf();

	D3D12_BLEND_DESC GetBlendState(BlendStateID id) const;

private:

	std::vector<D3D12_BLEND_DESC> blendStates_;

};
