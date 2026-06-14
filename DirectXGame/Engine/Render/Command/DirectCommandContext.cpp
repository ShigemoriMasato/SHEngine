#include "DirectCommandContext.h"
#include "DirectCommandContext.h"

using namespace SHEngine;

void DirectCommandContext::Initialize(DXDevice* device, int initCmdObjNum) {
	PrivateInitialize(device, Command::Type::Direct, initCmdObjNum);
}
