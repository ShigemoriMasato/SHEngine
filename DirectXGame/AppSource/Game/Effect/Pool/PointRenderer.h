#pragma once
#include <Compute/ComputeObject.h>
#include <Render/Renderer.h>
#include <Render/Screen/Display.h>

class PointRenderer {
public:

	void Initialize(const Pool& pool, SHEngine::TextureManager* textureManager, SHEngine::Screen::IDisplay* renderTarget, const SHEngine::DrawData& pedd);
	void Update(const Matrix4x4& vpMatrix);
	void Draw(DCC* dcc, CCC* ccc);

private:

	std::unique_ptr<SHEngine::BufferContainer> container_;
	std::vector<std::unique_ptr<SHEngine::ComputeObject>> renderers_;
	std::unique_ptr<SHEngine::Renderer> postEffect_;

	std::vector<std::unique_ptr<SHEngine::Screen::Display>> intervalDisp_;

	SHEngine::GPUBuffer* redBuffer_ = nullptr;
	SHEngine::GPUBuffer* greenBuffer_ = nullptr;
	SHEngine::GPUBuffer* blueBuffer_ = nullptr;
	SHEngine::GPUBuffer* vpMatrixBuffer_ = nullptr;
	SHEngine::GPUBuffer* depthBuffer_ = nullptr;
};
