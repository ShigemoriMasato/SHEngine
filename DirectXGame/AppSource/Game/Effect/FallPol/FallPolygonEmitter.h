#pragma once
#include "../Polygon/PolygonEmitter.h"

class FallPolygonEmitter : public PolygonEmitter {
public:

	struct MeshInfo {
		std::string modelPath = "";
		Transform transform = {};
		Vector4 color = { 1,1,1,1 };
		uint32_t emitNum = 0;
	};

	struct MeshList {
		void Save(BinaryManager& bin) const;
		void Load(BinaryManager& bin);

		std::vector<MeshInfo> meshes;
	};

	struct Sphere {
		Vector3 pos;
		float radius;
	};

public:

	void Initialize(SHEngine::Engine* engine, const Pool& pool) override;
	void Update(CCC* compute, float deltaTime) override;

	void AddPolygon(const MeshList& meshList, SHEngine::ModelManager* modelManager);

	void SetConfig(Config& config) override;

	void Fall(Sphere sphere);

	void SetGravity(Vector3 gravity) {
		gravity_->CopyBuffer(&gravity, sizeof(Vector3));
	}
	void SetLifeTime(float lifeTime) {
		lifeTime_->CopyBuffer(&lifeTime, sizeof(float));
	}

private:

	std::unique_ptr<SHEngine::ComputeObject> fallSphere_ = nullptr;

	SHEngine::GPUBuffer* isFall_ = nullptr;
	SHEngine::GPUBuffer* gravity_ = nullptr;

	SHEngine::GPUBuffer* spheres_ = nullptr;
	SHEngine::GPUBuffer* sphereNum_ = nullptr;

	const uint32_t kMaxSphereNum_ = 64;


	std::vector<Sphere> sphereList_ = {};
};
