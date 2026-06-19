#include "DirectCommandContext.h"
#include "DirectCommandContext.h"

using namespace SHEngine;

void DirectCommandContext::Initialize(DXDevice* device, int initCmdObjNum) {
	PrivateInitialize(device, Command::Type::Direct, initCmdObjNum);
}

void SHEngine::DirectCommandContext::SetRenderTarget(Screen::IDisplay* display, bool clear) {
	renderTarget_ = display;
	GetCurrentCmdObj()->SetRenderTarget(display, clear);
}
