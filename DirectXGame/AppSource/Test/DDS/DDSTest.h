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
	SHEngine::GPUBuffer* wvpCubeBuffer_;

	std::unique_ptr<SHEngine::Renderer> reflectObj_;
	struct VSData {
		Matrix4x4 world;
		Matrix4x4 wvp;
	}vsData_;
	SHEngine::GPUBuffer* vsBuffer_;

	struct PSData {
		Vector3 cameraPos;
		float strength;
	}psData_;
	SHEngine::GPUBuffer* psBuffer_;

	Vector3 position_;
	Vector3 rotate_;
	float timer_;
};
