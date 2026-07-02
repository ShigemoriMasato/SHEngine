#pragma once
#include <Render/Renderer.h>
#include <SHEngine.h>

class Cylinder {
public:

	Cylinder(SHEngine::Engine* engine);

	void Update(float deltaTime, Matrix4x4 view);
	void Draw(DCC* dcc);

	void DrawImGui();

private:

	std::unique_ptr<SHEngine::BufferContainer> container_;
	std::unique_ptr<SHEngine::Renderer> renderer_;

	SHEngine::GPUBuffer* wvpBuffer_ = nullptr;
	SHEngine::GPUBuffer* colorBuffer_ = nullptr;

	Transform transform_;
	Vector4 color_ = { 1.0f, 1.0f, 1.0f, 1.0f };
};
