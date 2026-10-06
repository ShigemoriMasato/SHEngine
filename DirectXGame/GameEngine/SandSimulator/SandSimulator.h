#pragma once
#include "Data/SandData.h"
#include "Renderer/SandRenderer.h"
#include "Processor/SandProcessor.h"
#include <Render/Command/DirectCommandContext.h>

namespace Sand {
	struct InitializeData {
		//砂1一粒分のモデル
		const ModelData* sandModel;

		uint32_t particleCount;

		uint32_t gridVerticalCount;
		uint32_t gridHorizontalCount;
		uint32_t gridDepthCount;
	};
}

class SandSimulator {
public:

	SandSimulator(const Sand::InitializeData& initData);

	void Initialize(SHEngine::ICommandContext* commandContext);
	void Update(float deltaTime, SHEngine::ICommandContext* commandContext);
	void Draw(DCC* dcc);
	void DebugDraw(DCC* dcc);

	void DrawImGui();

	void SetCamera(const Camera* camera);

private:

	std::unique_ptr<SHEngine::BufferContainer> container_ = nullptr;

	Sand::Particle particleData_;
	Sand::Grid gridData_;
	Sand::FreeList freeList_;

	std::unique_ptr<Sand::Renderer> renderer_ = nullptr;
	std::unique_ptr<Sand::Processor> processor_ = nullptr;

	float scale;
	Vector4 color;
};
