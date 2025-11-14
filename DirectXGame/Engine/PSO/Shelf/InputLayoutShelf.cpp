#include "InputLayoutShelf.h"

InputLayoutShelf::InputLayoutShelf() {

	inputLayouts_.resize(int(InputLayoutID::Count));
    inputElementsList_.resize(static_cast<int>(InputLayoutID::Count));
    {
        inputElementsList_[int(InputLayoutID::Default)].resize(4);

        //InputLayout
        inputElementsList_[int(InputLayoutID::Default)][0].SemanticName = "POSITION";
        inputElementsList_[int(InputLayoutID::Default)][0].SemanticIndex = 0;
        inputElementsList_[int(InputLayoutID::Default)][0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
        inputElementsList_[int(InputLayoutID::Default)][0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

        inputElementsList_[int(InputLayoutID::Default)][1].SemanticName = "TEXCOORD";
        inputElementsList_[int(InputLayoutID::Default)][1].SemanticIndex = 0;
        inputElementsList_[int(InputLayoutID::Default)][1].Format = DXGI_FORMAT_R32G32_FLOAT;
        inputElementsList_[int(InputLayoutID::Default)][1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

        inputElementsList_[int(InputLayoutID::Default)][2].SemanticName = "NORMAL";
        inputElementsList_[int(InputLayoutID::Default)][2].SemanticIndex = 0;
        inputElementsList_[int(InputLayoutID::Default)][2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
        inputElementsList_[int(InputLayoutID::Default)][2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

        inputElementsList_[int(InputLayoutID::Default)][3].SemanticName = "TEXTURE";
        inputElementsList_[int(InputLayoutID::Default)][3].SemanticIndex = 0;
        inputElementsList_[int(InputLayoutID::Default)][3].Format = DXGI_FORMAT_R32_UINT;
        inputElementsList_[int(InputLayoutID::Default)][3].InputSlot = 1;
        inputElementsList_[int(InputLayoutID::Default)][3].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

        D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
        inputLayoutDesc.pInputElementDescs = inputElementsList_[int(InputLayoutID::Default)].data();
        inputLayoutDesc.NumElements = static_cast<UINT>(inputElementsList_[int(InputLayoutID::Default)].size());

        inputLayouts_[int(InputLayoutID::Default)] = inputLayoutDesc;
    }

    {
        inputElementsList_[int(InputLayoutID::Model)].resize(5);

        //InputLayout
        inputElementsList_[int(InputLayoutID::Model)][0].SemanticName = "POSITION";
        inputElementsList_[int(InputLayoutID::Model)][0].SemanticIndex = 0;
        inputElementsList_[int(InputLayoutID::Model)][0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
        inputElementsList_[int(InputLayoutID::Model)][0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

        inputElementsList_[int(InputLayoutID::Model)][1].SemanticName = "TEXCOORD";
        inputElementsList_[int(InputLayoutID::Model)][1].SemanticIndex = 0;
        inputElementsList_[int(InputLayoutID::Model)][1].Format = DXGI_FORMAT_R32G32_FLOAT;
        inputElementsList_[int(InputLayoutID::Model)][1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

        inputElementsList_[int(InputLayoutID::Model)][2].SemanticName = "NORMAL";
        inputElementsList_[int(InputLayoutID::Model)][2].SemanticIndex = 0;
        inputElementsList_[int(InputLayoutID::Model)][2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
        inputElementsList_[int(InputLayoutID::Model)][2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

        inputElementsList_[int(InputLayoutID::Model)][3].SemanticName = "TEXTURE";
        inputElementsList_[int(InputLayoutID::Model)][3].SemanticIndex = 0;
        inputElementsList_[int(InputLayoutID::Model)][3].Format = DXGI_FORMAT_R32_UINT;
        inputElementsList_[int(InputLayoutID::Model)][3].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

		inputElementsList_[int(InputLayoutID::Model)][4].SemanticName = "NODEINDEX";
		inputElementsList_[int(InputLayoutID::Model)][4].SemanticIndex = 0;
		inputElementsList_[int(InputLayoutID::Model)][4].Format = DXGI_FORMAT_R32_SINT;
		inputElementsList_[int(InputLayoutID::Model)][4].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

        D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
        inputLayoutDesc.pInputElementDescs = inputElementsList_[int(InputLayoutID::Model)].data();
        inputLayoutDesc.NumElements = static_cast<UINT>(inputElementsList_[int(InputLayoutID::Model)].size());

        inputLayouts_[int(InputLayoutID::Model)] = inputLayoutDesc;
    }

}

InputLayoutShelf::~InputLayoutShelf() {
}

D3D12_INPUT_LAYOUT_DESC InputLayoutShelf::GetInputLayoutDesc(InputLayoutID id) const {
	return inputLayouts_[int(id)];
}
