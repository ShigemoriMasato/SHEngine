#include "ComputeCommandContext.h"

void SHEngine::ComputeCommandContext::Initialize(DXDevice* device, int initCmdObjNum) {
	PrivateInitialize(device, Command::Type::Compute, initCmdObjNum);
}
