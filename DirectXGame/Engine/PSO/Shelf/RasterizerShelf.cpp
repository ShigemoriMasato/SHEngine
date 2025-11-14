#include "RasterizerShelf.h"

RasterizerShelf::RasterizerShelf() {
	rasterizerDescs_.resize(int(RasterizerID::Count));

    //RasisterzerStateの設定
    D3D12_RASTERIZER_DESC defaultDesc{};
    //裏面(時計回り)を表示しない
    defaultDesc.CullMode = D3D12_CULL_MODE_BACK;
    //三角形の中を塗りつぶす
    defaultDesc.FillMode = D3D12_FILL_MODE_SOLID;

    rasterizerDescs_[int(RasterizerID::Fill)] = defaultDesc;

    //RasisterzerStateの設定
    D3D12_RASTERIZER_DESC wireframeDesc{};
    //裏面(時計回り)を表示しない
    wireframeDesc.CullMode = D3D12_CULL_MODE_BACK;
    //三角形の中を塗りつぶす
    wireframeDesc.FillMode = D3D12_FILL_MODE_SOLID;

    rasterizerDescs_[int(RasterizerID::Wireframe)] = wireframeDesc;
}

RasterizerShelf::~RasterizerShelf() {
}

D3D12_RASTERIZER_DESC RasterizerShelf::GetRasterizerDesc(RasterizerID id) const {
	return rasterizerDescs_[int(id)];
}
