#include "BlendStateShelf.h"

BlendStateShelf::BlendStateShelf() {
	blendStates_.resize(int(BlendStateID::Count));

	//NormalBlend
	D3D12_BLEND_DESC alphaBlend{};
	alphaBlend.AlphaToCoverageEnable = false;
	alphaBlend.IndependentBlendEnable = false;
	alphaBlend.RenderTarget[0].BlendEnable = true;
	alphaBlend.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	alphaBlend.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	alphaBlend.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	alphaBlend.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	alphaBlend.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	alphaBlend.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	//すべての色を取り込む
	alphaBlend.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_RED | D3D12_COLOR_WRITE_ENABLE_BLUE | D3D12_COLOR_WRITE_ENABLE_GREEN;
	blendStates_[int(BlendStateID::Normal)] = alphaBlend;

	//AddBlend
	D3D12_BLEND_DESC addBlend{};
	addBlend.AlphaToCoverageEnable = false;
	addBlend.IndependentBlendEnable = false;
	addBlend.RenderTarget[0].BlendEnable = true;
	addBlend.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	addBlend.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	addBlend.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
	addBlend.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	addBlend.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	addBlend.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	//すべての色を取り込む
	addBlend.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_RED | D3D12_COLOR_WRITE_ENABLE_BLUE | D3D12_COLOR_WRITE_ENABLE_GREEN;
	blendStates_[int(BlendStateID::Add)] = addBlend;

	//SubtractBlend
	D3D12_BLEND_DESC subtractBlend{};
	subtractBlend.AlphaToCoverageEnable = false;
	subtractBlend.IndependentBlendEnable = false;
	subtractBlend.RenderTarget[0].BlendEnable = true;
	subtractBlend.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	subtractBlend.RenderTarget[0].BlendOp = D3D12_BLEND_OP_REV_SUBTRACT;
	subtractBlend.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
	subtractBlend.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	subtractBlend.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	subtractBlend.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	//すべての色を取り込む
	subtractBlend.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_RED | D3D12_COLOR_WRITE_ENABLE_BLUE | D3D12_COLOR_WRITE_ENABLE_GREEN;
	blendStates_[int(BlendStateID::Subtract)] = subtractBlend;

	//MultiplyBlend
	D3D12_BLEND_DESC multiplyBlend{};
	multiplyBlend.AlphaToCoverageEnable = false;
	multiplyBlend.IndependentBlendEnable = false;
	multiplyBlend.RenderTarget[0].BlendEnable = true;
	multiplyBlend.RenderTarget[0].SrcBlend = D3D12_BLEND_ZERO;
	multiplyBlend.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	multiplyBlend.RenderTarget[0].DestBlend = D3D12_BLEND_SRC_COLOR;
	multiplyBlend.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	multiplyBlend.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	multiplyBlend.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	//すべての色を取り込む
	multiplyBlend.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_RED | D3D12_COLOR_WRITE_ENABLE_BLUE | D3D12_COLOR_WRITE_ENABLE_GREEN;
	blendStates_[int(BlendStateID::Multiply)] = multiplyBlend;

	//ScreenBlend
	D3D12_BLEND_DESC screenBlend{};
	screenBlend.AlphaToCoverageEnable = false;
	screenBlend.IndependentBlendEnable = false;
	screenBlend.RenderTarget[0].BlendEnable = true;
	screenBlend.RenderTarget[0].SrcBlend = D3D12_BLEND_INV_DEST_COLOR;
	screenBlend.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	screenBlend.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
	screenBlend.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	screenBlend.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	screenBlend.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	//すべての色を取り込む
	screenBlend.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_RED | D3D12_COLOR_WRITE_ENABLE_BLUE | D3D12_COLOR_WRITE_ENABLE_GREEN;
	blendStates_[int(BlendStateID::Screen)] = screenBlend;

}

BlendStateShelf::~BlendStateShelf() {
}

D3D12_BLEND_DESC BlendStateShelf::GetBlendState(BlendStateID id) const {
	return blendStates_[int(id)];
}
