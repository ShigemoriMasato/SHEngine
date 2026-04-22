#pragma once
#include <Compute/ComputeObject.h>
#include <Render/Renderer.h>

struct Pool {
	SHEngine::GPUBuffer* freeList;
	SHEngine::GPUBuffer* freeListIndex;
	SHEngine::GPUBuffer* position;
	SHEngine::GPUBuffer* color;
	SHEngine::GPUBuffer* type;
	SHEngine::GPUBuffer* particleNum;
	SHEngine::GPUBuffer* deltaTime;
	int maxParticleNum;
};

class ParticlePool {
public:

	//パーティクルのプールを初期化する。GPUBufferを作成して、DrawDataにセットする
	void Initialize(SHEngine::DrawData& planeDrawData, const int kMaxParticleNum, CmdObj* cmdObj);

	//全てのWorldMatrixにcamera行列をかけて、GPUに転送する
	void Update(const Matrix4x4& vpMatrix, const Matrix4x4& billboardMatrix, float deltaTime);

	//パーティクルの描画
	void Draw(CmdObj* cmdObj);

	//パーティクルの共通要素に関する操作
	void DrawImGui();

	const Pool& GetPool() const { return pool_; }

private:

	constexpr static inline int kThreadGroupSize_ = 256;

	std::unique_ptr<SHEngine::BufferContainer> container_;

	std::unique_ptr<SHEngine::ComputeObject> initialize_;
	std::unique_ptr<SHEngine::ComputeObject> update_;

	SHEngine::GPUBuffer* vpMatrixBuffer_ = nullptr;
	SHEngine::GPUBuffer* sizeBuffer_ = nullptr;
	Pool pool_{};

	std::unique_ptr<SHEngine::Renderer> renderer_;

	float size_ = 0.2f;
	struct Camera {
		Matrix4x4 vpMatrix;
		Matrix4x4 billboardMatrix;
	}camera_;
};
