#pragma once
#include <Game/Effect/Base/IEmitter.h>

class PointEmitter : public IEmitter {
public:

	void Initialize(SHEngine::Engine* engine, const Pool& pool) override;
	void Update(CCC* compute, float deltaTime) override;

private:



};
