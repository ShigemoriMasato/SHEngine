#include "AnimationModel.h"

void AnimationModel::StaticInitialize(SHEngine::ModelManager* modelManager, SHEngine::DrawDataManager* drawDataManager) {
	modelManager_ = modelManager;
	drawDataManager_ = drawDataManager;
}

void AnimationModel::Initialize(std::string filePath) {
}

bool AnimationModel::SetAnimation(std::string filePath, std::string animationName) {
	return false;
}

bool AnimationModel::SetAnimation(Animation& animation) {
	return false;
}

std::vector<Matrix4x4> AnimationModel::AcceptAnimation(float& timer) {
	return std::vector<Matrix4x4>();
}
