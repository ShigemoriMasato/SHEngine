#pragma once
#include <Assets/Model/ModelManager.h>
#include <Render/DrawData.h>

class AnimationModel {
public:

	void StaticInitialize(SHEngine::ModelManager* modelManager);

	// @brief アニメーションモデルの初期化
	// @param filePath モデルファイルのパス
	// @param canDraw 描画できるところまで自動で作成するか
	void Initialize(std::string filePath, bool canDraw = true, bool createDrawData = true);

	std::vector<SHEngine::DrawData> GetDrawData() const { return drawData_; }

	bool SetAnimation(std::string filePath, std::string animationName);
	bool SetAnimation(Animation& animation);

	void Update(float& animationTimer);

private:

	struct CalcedJoint {
		Matrix4x4 localMatrix;
		uint32_t parentIndex;
	};

	struct WellForGPU {
		Matrix4x4 skeletonSpaceMatrix;
		Matrix4x4 skeletonSpaceInverseTransposedMatrix;
	};

private:

	Matrix4x4 AnimationUpdate(const NodeAnimation& animation, float time);

private:

	static inline SHEngine::ModelManager* modelManager_ = nullptr;

	std::unique_ptr<SHEngine::BufferContainer> container_ = nullptr;
	std::vector<SHEngine::DrawData> drawData_;
	std::vector<std::vector<SHEngine::GPUBuffer*>> vertexBuffers_;
	std::vector<SHEngine::GPUBuffer*> indexBuffers_;

	const ModelData* modelData_{};

	Animation animation_{};

	std::vector<CalcedJoint> calcedJoints_{};
	std::vector<WellForGPU> wellForGPU_{};

	bool hasBone_ = false;
};
