#pragma once
#include <SHEngine.h>
#include <Render/Renderer.h>
#include <Camera/Camera.h>

class DDSTest {
public:

	void Initialize(SHEngine::Engine* engine);
	void Update(float deltaTime, Camera* camera);
	void Draw(CmdObj* cmdObj);

private:

	std::unique_ptr<SHEngine::BufferContainer> container_;

	std::unique_ptr<SHEngine::Renderer> ddsCube_;
	Vector3 scale_;
	SHEngine::GPUBuffer* wvpBuffer_;

};
