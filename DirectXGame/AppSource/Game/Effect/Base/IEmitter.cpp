#include "IEmitter.h"

uint32_t IEmitter::GetRandU() {
	return uintDist_(randomEngine_);
}

float IEmitter::GetRandF() {
    return floatDist_(randomEngine_);
}
