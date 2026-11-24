#pragma once
#include <Particle/Emitter.h>
#include <Assets/Model/ModelData.h>

namespace Particle {

	void EmitInnnerModel(Emitter* emitter, const Matrix4x4& transform, int emitCount, const ModelData* modelData);

};
