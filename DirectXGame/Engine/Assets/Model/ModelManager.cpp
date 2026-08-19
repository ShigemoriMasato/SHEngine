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
	LoadModel("Assets/.EngineResource/Model/Camera");
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

const ModelData* SHEngine::ModelManager::GetModelData(TestModel model) {
	return modelData_[static_cast<int>(model)].get();
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
	result.meshes = ModelLoader::LoadMeshes(scene);
	result.materials = ModelLoader::LoadMaterials(scene, filePath, textureManager_);
	result.skeleton = ModelLoader::CreateSkeleton(result.nodes, scene);

	return result;
}
