#include "ModelManager.h"

#include <Utility/SearchFile.h>
#include <Utility/Easing.h>
#include <Utility/MatrixFactory.h>
#include "ModelLoader.h"

#include <cassert>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

using namespace SHEngine;

namespace {
	std::string ExtensionSearcher(std::string directoryPath, std::vector<std::string> ext) {
		if (ext.empty()) {
			return "";
		}

		auto files = SearchFiles(directoryPath, ext.front());
		if (files.size() >= 1) {
			return files.front();
		}

		ext.erase(ext.begin());
		return ExtensionSearcher(directoryPath, ext);
	}
}

void ModelManager::Initialize(TextureManager* textureManager) {
	modelFilePaths_.clear();
	modelData_.clear();
	textureManager_ = textureManager;

	logger_ = GetLogger("Engine");

	LoadModel("Assets/.EngineResource/Model/Cube");
	LoadModel("Assets/.EngineResource/Model/Plane");
	LoadModel("Assets/.EngineResource/Model/Sphere");
	LoadModel("Assets/.EngineResource/Model/SimpleSkin");
	LoadModel("Assets/.EngineResource/Model/DefaultDesc");
}

const ModelData* ModelManager::LoadModel(std::string filePath) {
	// ファイルパスの確認と修正
	std::string fileName = FilePathChecker(filePath);

	// ファイル名が見つからない場合
	if (fileName.empty()) {
		logger_->error("Model file not found in directory: {}", filePath);
		return 0; // キューブのIDを返す
	}

	// すでに読み込んでいたらIDを返す
	const auto it = modelFilePaths_.find(filePath);
	if (it != modelFilePaths_.end()) {
		logger_->debug("Model already loaded: {}", filePath);
		return modelData_[it->second].get();
	}

	logger_->info("Loading Model: {}/{}", filePath, fileName);

	//Assimp
	Assimp::Importer importer;
	std::string path = (filePath + "/" + fileName);
	const aiScene* scene = nullptr;
	scene = importer.ReadFile(path.c_str(), aiProcess_MakeLeftHanded | aiProcess_FlipWindingOrder | aiProcess_FlipUVs | aiProcess_Triangulate);
	if (!scene) {
		logger_->error("Failed to load model: {} - Error: {}", path, importer.GetErrorString());
		return 0; // キューブのIDを返す
	}

	//読み込み
	int id = int(modelData_.size());
	auto& modelData = modelData_.emplace_back(std::make_unique<ModelData>());
	*modelData = CreateModelData(scene, filePath);
	modelFilePaths_[filePath] = id;

	return modelData.get();
}

void ModelManager::LoadAllModels() {
	auto files = SearchDirectoryNames("Assets/Model/");

	for (const auto& filePath : files) {
		LoadModel(filePath);
	}
}

Animation ModelManager::LoadAnimation(std::string filePath, std::string animationName) {

	std::string fileName = FilePathChecker(filePath);

	{
		auto it = animations_.find(filePath);
		if (it != animations_.end()) {
			logger_->debug("Animation already loaded: {}", filePath);

			if (it->second.empty()) {
				logger_->error("No animations found in file: {}", filePath);
				assert(false && "ModelManager::LoadAnimation: No animations found");
				return Animation{};
			}

			if (animationName.empty()) {
				return it->second.begin()->second;
			}

			auto it2 = it->second.find(animationName);
			if (it2 == it->second.end()) {
				logger_->error("Animation not found: {} in file: {}", animationName, filePath);
				assert(false && "ModelManager::LoadAnimation: Animation not found");
				return Animation{};
			}
			return it2->second;
		}
	}

	//Assimp
	Assimp::Importer importer;
	std::string path = (filePath + "/" + fileName);
	const aiScene* scene = nullptr;
	scene = importer.ReadFile(path.c_str(), aiProcess_MakeLeftHanded | aiProcess_FlipWindingOrder | aiProcess_FlipUVs | aiProcess_Triangulate);
	assert(scene && "ModelManager::LoadModel: Failed to load model");

	//読み込み
	auto animations = ModelLoader::LoadAnimations(scene);

	if (animations.empty()) {
		logger_->error("No animations found in file: {}", filePath);
		assert(false && "ModelManager::LoadAnimation: No animations found");
		return Animation{};
	}

	animations_[filePath] = animations;

	if (animationName.empty()) {
		return animations.begin()->second;
	}

	auto it = animations_[filePath].find(animationName);
	if (it == animations_[filePath].end()) {
		logger_->error("Animation not found: {} in file: {}", animationName, filePath);
		assert(false && "ModelManager::LoadAnimation: Animation not found");
		return Animation{};
	}

	return it->second;
}

std::string ModelManager::FilePathChecker(std::string& filePath) {
	//Assets/から始まっているか確認(Assets/Modelの可能性もあるのでAssets/のみ確認)
	std::string formatFirst = "Assets/";
	std::string factFilePath = "";
	if (filePath.length() < formatFirst.length()) {
		factFilePath = "Assets/Model/" + filePath;
	} else {
		for (int i = 0; i < formatFirst.length(); ++i) {
			if (filePath[i] != formatFirst[i]) {
				factFilePath = "Assets/Model/" + filePath;
				break;
			}

			if (i == formatFirst.length() - 1) {
				factFilePath = filePath;
			}
		}
	}
	filePath = factFilePath;

	std::vector<std::string> extensions = { ".fbx", ".obj", ".gltf", ".glb" };
	std::string fileName = ExtensionSearcher(filePath, extensions);

	return fileName;
}

ModelData SHEngine::ModelManager::CreateModelData(const aiScene* scene, std::string filePath) {
	ModelData result;
	//コードがごちゃつくのでModelLoaderに処理を投げる
	result.nodes = ModelLoader::LoadNodes(scene);

	return result;
}

Matrix4x4 AnimationUpdate(const Animation& animation, float time, const Node& node) {
	Vector3 position = CalculateValue(animation.nodeAnimations.at(node.name).position.keyframes, time);
	Quaternion rotation = CalculateValue(animation.nodeAnimations.at(node.name).rotate.keyframes, time);
	Vector3 scale = CalculateValue(animation.nodeAnimations.at(node.name).scale.keyframes, time);
	return Matrix::MakeScaleMatrix(scale) * rotation.ToMatrix() * Matrix::MakeTranslationMatrix(position);
}

std::vector<> AnimationUpdate(const Animation& animation, float time, const Skeleton& skeleton) {
	for(const Joint& joint : skeleton.joints) {
		if(auto it = animation.nodeAnimations.find(joint.name); it != animation.nodeAnimations.end()) {
			Vector3 position = CalculateValue(it->second.position.keyframes, time);
			Quaternion rotation = CalculateValue(it->second.rotate.keyframes, time);
			Vector3 scale = CalculateValue(it->second.scale.keyframes, time);
			joint.transform.position = position;
			joint.transform.rotate = rotation;
			joint.transform.scale = scale;
		}
	}
}

void SkeletonUpdate(Skeleton& skeleton) {
	for (Joint& joint : skeleton.joints) {
		joint.localMatrix = Matrix::MakeScaleMatrix(joint.transform.scale) *
			joint.transform.rotate.ToMatrix() *
			Matrix::MakeTranslationMatrix(joint.transform.position);

		if (joint.parent) {
			joint.skeletonSpaceMatrix = joint.localMatrix * skeleton.joints[*joint.parent].skeletonSpaceMatrix;
		} else {
			joint.skeletonSpaceMatrix = joint.localMatrix * skeleton.rootMatrix;
		}
	}
}

void SkinningUpdate(std::vector<WellForGPU>& result, const std::map<std::string, Skin>& skinCluster, const Skeleton& skeleton) {
	result.resize(skeleton.joints.size());
	for (size_t jointIndex = 0; jointIndex < skeleton.joints.size(); ++jointIndex) {
		assert(jointIndex < skeleton.joints.size());
		std::string key = skeleton.joints[jointIndex].name;
		result[jointIndex].skeletonSpaceMatrix = 
			skinCluster.at(key).inverseBindPoseMatrix * skeleton.joints[jointIndex].skeletonSpaceMatrix;

		result[jointIndex].skeletonSpaceInverseTransposeMatrix =
			Matrix::TransMatrix(result[jointIndex].skeletonSpaceMatrix.Inverse());
	}
}

Vector3 CalculateValue(const std::vector<KeyframeVector3>& keyframes, float time) {
	assert(!keyframes.empty());
	if (keyframes.size() == 1 || time <= keyframes[0].time) {
		return keyframes[0].value;
	}

	for(size_t index = 0; index < keyframes.size() - 1; ++index) {
		size_t nextIndex = index + 1;

		if (keyframes[index].time <= time && time <= keyframes[nextIndex].time) {
			float t = (time - keyframes[index].time) / (keyframes[nextIndex].time - keyframes[index].time);
			return lerp(keyframes[index].value, keyframes[nextIndex].value, t);
		}
	}

	return keyframes.back().value;
}

Quaternion CalculateValue(const std::vector<KeyframeQuaternion>& keyframes, float time) {
	assert(!keyframes.empty());
	if (keyframes.size() == 1 || time <= keyframes[0].time) {
		return keyframes[0].value;
	}

	for (size_t index = 0; index < keyframes.size() - 1; ++index) {
		size_t nextIndex = index + 1;

		if (keyframes[index].time <= time && time <= keyframes[nextIndex].time) {
			float t = (time - keyframes[index].time) / (keyframes[nextIndex].time - keyframes[index].time);
			return lerp(keyframes[index].value, keyframes[nextIndex].value, t);
		}
	}

	return keyframes.back().value;
}
