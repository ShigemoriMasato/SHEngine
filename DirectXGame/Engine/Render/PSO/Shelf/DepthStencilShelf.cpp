#include "DepthStencilShelf.h"

using namespace SHEngine::PSO;

/*
自分用メモ

基本的に近ければ近いほど0に近づく。0が最接近

Src..今から書くやつ
Dest..もともと書いてあるやつ

Src (op) Dest

Less	..近ければ近いほど <
Equal	..等しい場合		=
Greater	..遠ければ遠いほど >

*/

DepthStencilShelf::DepthStencilShelf() {

	depthStencilDescs_.resize(int(DepthStencilID::Count));

	D3D12_DEPTH_STENCIL_DESC noneDesc{};
	noneDesc.DepthEnable = false;	//深度バッファを使わない
	noneDesc.StencilEnable = FALSE; // ステンシルテストを使わないなら FALSE

	depthStencilDescs_[int(DepthStencilID::None)] = noneDesc;

	D3D12_DEPTH_STENCIL_DESC defaultDesc{};
	defaultDesc.DepthEnable = true;	//深度バッファを使う
	defaultDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;	//全ての深度値を使う
	defaultDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;	//深度値の比較方法

	defaultDesc.StencilEnable = FALSE; // ステンシルテストを使わないなら FALSE
	defaultDesc.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK; // デフォルト値
	defaultDesc.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK; // デフォルト値

	depthStencilDescs_[int(DepthStencilID::Default)] = defaultDesc;

	D3D12_DEPTH_STENCIL_DESC transparentDesc{};
	transparentDesc.DepthEnable = true;	//深度バッファを使う
	transparentDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;	//深度地を書き込まない
	transparentDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;	//深度値の比較方法

	transparentDesc.StencilEnable = FALSE; // ステンシルテストを使わないなら FALSE
	transparentDesc.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK; // デフォルト値
	transparentDesc.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK; // デフォルト値

	depthStencilDescs_[int(DepthStencilID::Transparent)] = transparentDesc;

}

DepthStencilShelf::~DepthStencilShelf() {
}

D3D12_DEPTH_STENCIL_DESC DepthStencilShelf::GetDepthStencilDesc(DepthStencilID id) const {
	return depthStencilDescs_[int(id)];
}
