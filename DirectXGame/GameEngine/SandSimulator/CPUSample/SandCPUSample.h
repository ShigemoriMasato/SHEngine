#pragma once
#include <SandSimulator/Renderer/SandRenderer.h>
#include <random>

class SandCPUSample {
public:

	SandCPUSample(uint32_t particleCount, const ModelData* modelData);

	void Initialize();
	void Update(float deltaTime);
	void Draw(DCC* dcc);

	void SetCamera(const Camera* camera) {
		renderer_->SetCamera(camera);
	}

private:

	std::vector<Vector3> positions_;
	std::vector<Vector3> velocities_;
	std::vector<Vector3> forces_;

	std::unique_ptr<SHEngine::BufferContainer> container_ = nullptr;
	Sand::Particle particleData_;
	std::unique_ptr<Sand::Renderer> renderer_ = nullptr;

	float scale_ = 0.05f;

	std::mt19937 randomEngine_;
};
