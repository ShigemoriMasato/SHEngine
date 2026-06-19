#include "BlendStateShelf.h"

using namespace SHEngine::PSO;

BlendStateShelf::BlendStateShelf() {
	blendStates_.resize(int(BlendStateID::Count));

	//NormalBlend
	D3D12_RENDER_TARGET_BLEND_DESC alphaBlend{};
	alphaBlend.BlendEnable = true;
	alphaBlend.SrcBlend = D3D12_BLEND_SRC_ALPHA;
	alphaBlend.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	alphaBlend.BlendOp = D3D12_BLEND_OP_ADD;
	alphaBlend.SrcBlendAlpha = D3D12_BLEND_ONE;
	alphaBlend.DestBlendAlpha = D3D12_BLEND_ZERO;
	alphaBlend.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	//すべての色を取り込む
	alphaBlend.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_RED | D3D12_COLOR_WRITE_ENABLE_BLUE | D3D12_COLOR_WRITE_ENABLE_GREEN;
	blendStates_[int(BlendStateID::Normal)] = alphaBlend;

	//AddBlend
	D3D12_RENDER_TARGET_BLEND_DESC addBlend{};
	addBlend.BlendEnable = true;
	addBlend.SrcBlend = D3D12_BLEND_SRC_ALPHA;
	addBlend.BlendOp = D3D12_BLEND_OP_ADD;
	addBlend.DestBlend = D3D12_BLEND_ONE;
	addBlend.SrcBlendAlpha = D3D12_BLEND_ONE;
	addBlend.DestBlendAlpha = D3D12_BLEND_ZERO;
	addBlend.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	//すべての色を取り込む
	addBlend.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_RED | D3D12_COLOR_WRITE_ENABLE_BLUE | D3D12_COLOR_WRITE_ENABLE_GREEN;
	blendStates_[int(BlendStateID::Add)] = addBlend;

	//SubtractBlend
	D3D12_RENDER_TARGET_BLEND_DESC subtractBlend{};
	subtractBlend.BlendEnable = true;
	subtractBlend.SrcBlend = D3D12_BLEND_SRC_ALPHA;
	subtractBlend.BlendOp = D3D12_BLEND_OP_REV_SUBTRACT;
	subtractBlend.DestBlend = D3D12_BLEND_ONE;
	subtractBlend.SrcBlendAlpha = D3D12_BLEND_ONE;
	subtractBlend.DestBlendAlpha = D3D12_BLEND_ZERO;
	subtractBlend.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	//すべての色を取り込む
	subtractBlend.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_RED | D3D12_COLOR_WRITE_ENABLE_BLUE | D3D12_COLOR_WRITE_ENABLE_GREEN;
	blendStates_[int(BlendStateID::Subtract)] = subtractBlend;

	//MultiplyBlend
	D3D12_RENDER_TARGET_BLEND_DESC multiplyBlend{};
	multiplyBlend.BlendEnable = true;
	multiplyBlend.SrcBlend = D3D12_BLEND_ZERO;
	multiplyBlend.BlendOp = D3D12_BLEND_OP_ADD;
	multiplyBlend.DestBlend = D3D12_BLEND_SRC_COLOR;
	multiplyBlend.SrcBlendAlpha = D3D12_BLEND_ONE;
	multiplyBlend.DestBlendAlpha = D3D12_BLEND_ZERO;
	multiplyBlend.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	//すべての色を取り込む
	multiplyBlend.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_RED | D3D12_COLOR_WRITE_ENABLE_BLUE | D3D12_COLOR_WRITE_ENABLE_GREEN;
	blendStates_[int(BlendStateID::Multiply)] = multiplyBlend;

	//ScreenBlend
	D3D12_RENDER_TARGET_BLEND_DESC screenBlend{};
	screenBlend.BlendEnable = true;
	screenBlend.SrcBlend = D3D12_BLEND_INV_DEST_COLOR;
	screenBlend.BlendOp = D3D12_BLEND_OP_ADD;
	screenBlend.DestBlend = D3D12_BLEND_ONE;
	screenBlend.SrcBlendAlpha = D3D12_BLEND_ONE;
	screenBlend.DestBlendAlpha = D3D12_BLEND_ZERO;
	screenBlend.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	//すべての色を取り込む
	screenBlend.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_RED | D3D12_COLOR_WRITE_ENABLE_BLUE | D3D12_COLOR_WRITE_ENABLE_GREEN;
	blendStates_[int(BlendStateID::Screen)] = screenBlend;

	//NoneBlend
	D3D12_RENDER_TARGET_BLEND_DESC noneBlend{};
	noneBlend.BlendEnable = false;
	blendStates_[int(BlendStateID::None)] = noneBlend;
}

BlendStateShelf::~BlendStateShelf() {
}

D3D12_BLEND_DESC BlendStateShelf::GetBlendState(BlendStateID* id) const {
	D3D12_BLEND_DESC blendDesc{};
	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.IndependentBlendEnable = false;

	//8個のBlendStateIDが来ている想定で動く
	for (int i = 0; i < 8; ++i) {
		blendDesc.RenderTarget[i] = blendStates_[int(id[i])];
	}

	return blendDesc;
}
