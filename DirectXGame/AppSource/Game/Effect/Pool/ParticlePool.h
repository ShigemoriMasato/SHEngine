#pragma once
#include <Compute/ComputeObject.h>
#include <Render/MeshRenderer.h>
#include <Render/Screen/Display.h>
#include <Camera/Camera.h>

struct Pool {
	SHEngine::GPUBuffer* freeList;
	SHEngine::GPUBuffer* freeListIndex;
	SHEngine::GPUBuffer* position;
	SHEngine::GPUBuffer* color;
	SHEngine::GPUBuffer* particleNum;
	SHEngine::GPUBuffer* deltaTime;
	int maxParticleNum;
};

class ParticlePool {
public:

	//パーティクルのプールを初期化する。GPUBufferを作成して、DrawDataにセットする
	void Initialize(const int kMaxParticleNum, CCC* compute, SHEngine::TextureManager* textureManager);

	//全てのWorldMatrixにcamera行列をかけて、GPUに転送する
	void Update(Camera* camera, float deltaTime, CCC* compute);

	//パーティクルの描画
	void Draw(DCC* dcc, CCC* ccc);

	//パーティクルの共通要素に関する操作
	void DrawImGui();

	const Pool& GetPool() const { return pool_; }

private:

	constexpr static inline int kThreadGroupSize_ = 1024;

	std::unique_ptr<SHEngine::BufferContainer> container_;

	std::unique_ptr<SHEngine::ComputeObject> initialize_;

	SHEngine::GPUBuffer* vpMatrixBuffer_ = nullptr;
	SHEngine::GPUBuffer* sizeBuffer_ = nullptr;
	Pool pool_{};

	std::unique_ptr<SHEngine::MeshRenderer> renderer_;

	float size_ = 0.01f;
	struct CameraData {
		Matrix4x4 vpMatrix;
		Matrix4x4 billboardMatrix;
		Vector3 cameraPos;
	}camera_;

	int drawCount_;
};
