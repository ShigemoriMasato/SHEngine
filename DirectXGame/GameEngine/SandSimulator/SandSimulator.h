#pragma once
#include "Data/SandData.h"
#include "Renderer/SandRenderer.h"
#include "Processor/SandProcessor.h"
#include "Debugger/SandConfigure.h"
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

	void Save(BinaryManager* bin) const;
	void Load(BinaryManager* bin);

private:

	std::unique_ptr<SHEngine::BufferContainer> container_ = nullptr;

	Sand::Particle particleData_;

	std::unique_ptr<Sand::Renderer> renderer_ = nullptr;
	std::unique_ptr<Sand::Processor> processor_ = nullptr;

	std::unique_ptr<Sand::Configure> configure_ = nullptr;

	Vector3 spherePos_ = { 0.0f, 0.0f, 0.0f };
	float sphereRadius_ = 1.0f;
	bool isInitialize_ = false;

	Vector4 color;
	float scale = 1.0f;
};
