#pragma once
#include <Assets/Model/ModelManager.h>

class AnimationModel {
public:

	void StaticInitialize(SHEngine::ModelManager* modelManager, SHEngine::DrawDataManager* drawDataManager);

	void Initialize(std::string filePath);

	std::vector<SHEngine::DrawData> GetDrawData() const { return drawData_; }

	bool SetAnimation(std::string filePath, std::string animationName);
	bool SetAnimation(Animation& animation);

	std::vector<Matrix4x4> AcceptAnimation(float& timer);

private:

	static inline SHEngine::ModelManager* modelManager_ = nullptr;
	static inline SHEngine::DrawDataManager* drawDataManager_ = nullptr;

	std::vector<SHEngine::DrawData> drawData_;

	NodeModelData nodeModelData_{};
	Animation animation_{};

};
