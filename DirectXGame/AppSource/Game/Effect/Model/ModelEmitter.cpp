#include "ModelEmitter.h"

VertexEmitter::VertexEmitter() : IEmitter(100000) {

}

void VertexEmitter::Initialize(SHEngine::Engine* engine, const Pool& pool, const uint32_t offset) {
}

int VertexEmitter::AddModel(const std::vector<Vector3>& vertices) {
	return 0;
}
