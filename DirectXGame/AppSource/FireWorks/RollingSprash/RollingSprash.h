#pragma once
#include <Particle/Emitter.h>
#include "SprashData.h"

class RollingSprash {
public:

	RollingSprash();
	~RollingSprash();

	void Initialize(const RollingSprashData& data);
	void Update();
	void Draw();

private:

	std::vector<Particle::Emitter> emitter_;

};
