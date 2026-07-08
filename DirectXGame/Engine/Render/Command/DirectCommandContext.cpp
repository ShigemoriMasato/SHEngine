#include "DirectCommandContext.h"
#include <Render/Screen/IDisplay.h>

using namespace SHEngine;

void DirectCommandContext::Initialize(DXDevice* device, int initCmdObjNum) {
	PrivateInitialize(device, Command::Type::Direct, initCmdObjNum);
}

void SHEngine::DirectCommandContext::SetRenderTarget(Screen::IDisplay* display, bool clear, bool setViewPort) {
	renderTarget_ = display;

	auto rtvHandle = display->GetRTVHandle();
	auto dsvHandle = display->GetDSVHandle();
	auto cmdList = GetCommandList();

	if (dsvHandle->ptr == 0) {
		dsvHandle = nullptr;
	}

	display->ToRenderTarget(this);

	cmdList->OMSetRenderTargets(display->GetRenderTargetNum(), rtvHandle, FALSE, dsvHandle);

	if (clear) {
		display->Clear(this);
	}

	if (setViewPort) {
		display->SetViewport(this);
	}
}
