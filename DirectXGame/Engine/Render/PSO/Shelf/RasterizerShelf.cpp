#include "RasterizerShelf.h"

using namespace SHEngine::PSO;

RasterizerShelf::RasterizerShelf() {
	rasterizerDescs_.resize(int(RasterizerID::Count));

	//RasterizerStateの設定
	D3D12_RASTERIZER_DESC defaultDesc{};
	//裏面(時計回り)を表示しない
	defaultDesc.CullMode = D3D12_CULL_MODE_BACK;
	//三角形の中を塗りつぶす
	defaultDesc.FillMode = D3D12_FILL_MODE_SOLID;

	rasterizerDescs_[int(RasterizerID::Fill)] = defaultDesc;

	//RasterizerStateの設定
	D3D12_RASTERIZER_DESC wireframeDesc{};
	//裏面(時計回り)を表示しない
	wireframeDesc.CullMode = D3D12_CULL_MODE_NONE;
	//三角形の中を塗りつぶす
	wireframeDesc.FillMode = D3D12_FILL_MODE_WIREFRAME;
	wireframeDesc.AntialiasedLineEnable = true;

	rasterizerDescs_[int(RasterizerID::Wireframe)] = wireframeDesc;

	//RasterizerStateの設定
	D3D12_RASTERIZER_DESC backframeDesc{};
	//表を表示しない
	backframeDesc.CullMode = D3D12_CULL_MODE_FRONT;
	//三角形の中を塗りつぶす
	backframeDesc.FillMode = D3D12_FILL_MODE_SOLID;

	rasterizerDescs_[int(RasterizerID::Back)] = backframeDesc;

	//RasterizerStateの設定
	D3D12_RASTERIZER_DESC cullNoneDesc{};
	//表を表示しない
	cullNoneDesc.CullMode = D3D12_CULL_MODE_NONE;
	//三角形の中を塗りつぶす
	cullNoneDesc.FillMode = D3D12_FILL_MODE_SOLID;

	rasterizerDescs_[int(RasterizerID::CullNone)] = cullNoneDesc;
}

RasterizerShelf::~RasterizerShelf() {
}

D3D12_RASTERIZER_DESC RasterizerShelf::GetRasterizerDesc(RasterizerID id) const {
	return rasterizerDescs_[int(id)];
}
