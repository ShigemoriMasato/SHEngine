#include "IDisplay.h"

void SHEngine::Screen::IDisplay::SetViewportInPrivate(DCC* dcc, Vector2 min, Vector2 size) {
	auto cmdList = dcc->GetCommandList();

	//ViewPortとScissorRectの設定
	D3D12_VIEWPORT viewPort{};
	viewPort.TopLeftX = min.x;
	viewPort.TopLeftY = min.y;
	viewPort.Width = size.x;
	viewPort.Height = size.y;
	viewPort.MinDepth = 0.0f;
	viewPort.MaxDepth = 1.0f;

	cmdList->RSSetViewports(1, &viewPort);

	D3D12_RECT scissorRect{};
	scissorRect.left = 0;
	scissorRect.top = 0;
	scissorRect.right = static_cast<LONG>(size.x - min.x);
	scissorRect.bottom = static_cast<LONG>(size.y - min.y);

	cmdList->RSSetScissorRects(1, &scissorRect);
}
