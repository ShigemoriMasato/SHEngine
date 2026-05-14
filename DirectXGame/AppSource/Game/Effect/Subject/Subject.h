#pragma once
#include <SHEngine.h>
#include <Render/Renderer.h>
#include <Camera/Camera.h>

class Subject {
public:

	void Initialize(SHEngine::Engine* engine);
	void Update(const Matrix4x4& vpMat);
	void Draw(CmdObj* cmdObj);

private:

	std::unique_ptr<SHEngine::BufferContainer> container_;

	std::unique_ptr<SHEngine::Renderer> cube_;
	SHEngine::GPUBuffer* wvp_;

};
