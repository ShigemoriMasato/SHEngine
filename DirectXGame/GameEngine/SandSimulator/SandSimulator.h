#pragma once
#include "Data/SandData.h"
#include <Core/Command/ICommandContext.h>
#include <Render/Command/DirectCommandContext.h>

namespace Sand {
	struct InitializeData {
		uint32_t particleCount;

		uint32_t gridVerticalCount;
		uint32_t gridHorizontalCount;
		uint32_t gridDepthCount;
	};

	struct AddSandConfig {
		uint32_t particleCount;
		Vector3 position;
		float radius;
	};
}

class SandSimulator {
public:

	SandSimulator(const Sand::InitializeData& initData);

	void Initialize(const SHEngine::ICommandContext* commandContext);
	void Update(float deltaTime, const SHEngine::ICommandContext* commandContext);
	void Draw(const DCC* dcc);
	void DebugDraw(const DCC* dcc);

	void AddSand(const Sand::AddSandConfig& config);

private:

	

};
