#include "FrameCounter.h"
#include <Render/Buffer/GPUBuffer.h>
#include <Core/Command/CommandObject.h>

using namespace SHEngine;

void FrameCounter::Initialize() {
	frame_ = 0;
}

void FrameCounter::Update() {
	++frame_;

	GPUBuffer::SetCurrentIndex(frame_);
	Command::Object::SetCurrentIndex(frame_);
}
