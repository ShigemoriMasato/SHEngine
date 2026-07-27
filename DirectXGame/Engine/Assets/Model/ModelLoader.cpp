#include "ModelLoader.h"
#include <Utility/MatrixFactory.h>
#include <Utility/MyMath.h>

using namespace SHEngine;

void ModelLoader::CreateNodes(const aiNode* node, std::vector<Node>& nodes, uint32_t parentIndex) {
	uint32_t nodeIndex = static_cast<uint32_t>(nodes.size());
	Node& newNode = nodes.emplace_back();

	//ローカル座標の読み込み
	aiVector3D scale, position;
	aiQuaternion rotate;
	node->mTransformation.Decompose(scale, rotate, position);
	newNode.localTransform.scale = { scale.x, scale.y, scale.z };
	newNode.localTransform.rotate = { rotate.x, rotate.y, rotate.z, rotate.w };
	newNode.localTransform.position = { position.x, position.y, position.z };
	newNode.localMatrix = Matrix::MakeScaleMatrix(newNode.localTransform.scale) *
		newNode.localTransform.rotate.ToMatrix() *
		Matrix::MakeTranslationMatrix(newNode.localTransform.position);

	newNode.name = node->mName.C_Str();

	newNode.parent = parentIndex;

	//メッシュ情報登録
	if (node->mNumMeshes > 0) {
		//メッシュは1ノードに1つしかない前提
		uint32_t meshIndex = node->mMeshes[0];
		newNode.meshIndex = meshIndex;
	}

	for (uint32_t childIndex = 0; childIndex < node->mNumChildren; ++childIndex) {
		//再帰的に読んで階層構造を作っていく
		CreateNodes(node->mChildren[childIndex], nodes, nodeIndex);
	}
}

std::vector<Node> ModelLoader::LoadNodes(const aiScene* scene) {
	int parentIndex = -1;
	std::vector<Node> nodes;
	aiNode* rootNode = scene->mRootNode;
	CreateNodes(rootNode, nodes, -1);
	return nodes;
}

std::vector<Mesh> ModelLoader::LoadMeshes(const aiScene* scene) {
	std::vector<Mesh> meshes;
	for (uint32_t mesh = 0; mesh < scene->mNumMeshes; ++mesh) {
		const aiMesh* ai_mesh = scene->mMeshes[mesh];
		Mesh newMesh{};
		newMesh.name = ai_mesh->mName.C_Str();
		newMesh.position = LoadPositions(ai_mesh);
		newMesh.normal = LoadNormals(ai_mesh);
		newMesh.texcoord = LoadTexcoords(ai_mesh);
		newMesh.color = LoadColors(ai_mesh);
		newMesh.vertexInfluences = LoadVertexInfluences(ai_mesh);
		newMesh.indices = LoadIndices(ai_mesh);
		newMesh.primitives = LoadPrimitives(ai_mesh);
		meshes.push_back(newMesh);
	}
	return meshes;
}

std::vector<Vector3> ModelLoader::LoadPositions(const aiMesh* ai_mesh) {
	std::vector<Vector3> positions;

	positions.resize(ai_mesh->mNumVertices);
	for (uint32_t v = 0; v < ai_mesh->mNumVertices; ++v) {
		//位置
		positions[v].x = ai_mesh->mVertices[v].x;
		positions[v].y = ai_mesh->mVertices[v].y;
		positions[v].z = ai_mesh->mVertices[v].z;
	}

	return positions;
}

std::vector<Vector3> ModelLoader::LoadNormals(const aiMesh* ai_mesh) {
	uint32_t vertexCount = ai_mesh->mNumVertices;
	std::vector<Vector3> normals(vertexCount, {});

	for (uint32_t v = 0; v < vertexCount; ++v) {
		//法線
		normals[v].x = ai_mesh->mNormals[v].x;
		normals[v].y = ai_mesh->mNormals[v].y;
		normals[v].z = ai_mesh->mNormals[v].z;
	}

	return normals;
}

std::vector<Vector2> ModelLoader::LoadTexcoords(const aiMesh* ai_mesh) {
	uint32_t vertexCount = ai_mesh->mNumVertices;
	std::vector<Vector2> texcoords(vertexCount, {});
	if (ai_mesh->GetNumUVChannels() == 0) {
		return texcoords;
	}

	for (uint32_t v = 0; v < vertexCount; ++v) {
		//UV
		texcoords[v].x = ai_mesh->mTextureCoords[0][v].x;
		texcoords[v].y = ai_mesh->mTextureCoords[0][v].y;
	}

	return texcoords;
}

std::vector<Vector4> ModelLoader::LoadColors(const aiMesh* ai_mesh) {
	uint32_t vertexCount = ai_mesh->mNumVertices;
	std::vector<Vector4> colors(vertexCount, {});
	if (ai_mesh->GetNumColorChannels() == 0) {
		return colors;
	}

	for (uint32_t v = 0; v < vertexCount; ++v) {
		//色情報
		colors[v].x = ai_mesh->mColors[0][v].r;
		colors[v].y = ai_mesh->mColors[0][v].g;
		colors[v].z = ai_mesh->mColors[0][v].b;
		colors[v].w = ai_mesh->mColors[0][v].a;
	}

	return colors;
}

std::vector<VertexInfluence> ModelLoader::LoadVertexInfluences(const aiMesh* ai_mesh) {
	std::vector<VertexInfluence> vertexInfluences;
	vertexInfluences.resize(ai_mesh->mNumVertices, {});

	for (uint32_t boneIndex = 0; boneIndex < ai_mesh->mNumBones; ++boneIndex) {
		aiBone* bone = ai_mesh->mBones[boneIndex];

		for (uint32_t weightIndex = 0; weightIndex < bone->mNumWeights; ++weightIndex) {
			aiVertexWeight weight = bone->mWeights[weightIndex];
			VertexInfluence& vertexInfluence = vertexInfluences[weight.mVertexId];

			//空いてるとこに挿入(オーバーしたら諦める)
			for (int i = 0; i < MAX_JOINTS_PER_VERTEX; ++i) {
				if (vertexInfluence.weight[i] == 0.0f) {
					vertexInfluence.weight[i] = weight.mWeight;
					vertexInfluence.joint[i] = boneIndex;
					break;
				}
			}
		}
	}
	return vertexInfluences;
}

std::vector<uint32_t> ModelLoader::LoadIndices(const aiMesh* ai_mesh) {
	std::vector<uint32_t> indices;
	
	for (uint32_t face = 0; face < ai_mesh->mNumFaces; ++face) {
		for (uint32_t i = 0; i < ai_mesh->mFaces[face].mNumIndices; ++i) {
			indices.push_back(ai_mesh->mFaces[face].mIndices[i]);
		}
	}
	return indices;
}

std::vector<Primitive> ModelLoader::LoadPrimitives(const aiMesh* ai_mesh) {
	std::vector<Primitive> primitives;

	primitives.resize(ai_mesh->mNumFaces);
	for (uint32_t f = 0; f < ai_mesh->mNumFaces; ++f) {
		Primitive primitive{};
		primitive.indexCount = ai_mesh->mFaces[f].mNumIndices;
		primitive.indexOffset = f * 3;
		primitive.materialIndex = ai_mesh->mMaterialIndex;
		primitives[f] = primitive;
	}

	return primitives;
}

std::vector<Material> ModelLoader::LoadMaterials(const aiScene* scene, std::string directoryPath, TextureManager* textureManager) {
	std::vector<Material> materials;

	for (uint32_t i = 0; i < scene->mNumMaterials; ++i) {
		aiMaterial* ai_material = scene->mMaterials[i];
		Material material{};

		//テクスチャ読み込み
		aiString texturePath;
		if (ai_material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == AI_SUCCESS) {
			std::string path = texturePath.C_Str();
			material.textureIndex = textureManager->LoadTexture(directoryPath + "/" + path);

			if (material.textureIndex == textureManager->GetErrorTextureHandle()) {
				// Assets/Texture/から探す
				material.textureIndex = textureManager->LoadTexture(path);
			}

		} else {
			material.textureIndex = textureManager->GetErrorTextureHandle();
		}

		aiString normalTexturePath;
		if (ai_material->GetTexture(aiTextureType_NORMALS, 0, &normalTexturePath) == AI_SUCCESS) {
			std::string path = normalTexturePath.C_Str();
			material.normalTexture = textureManager->LoadTexture(directoryPath + "/" + path);
		} else {
			material.normalTexture = textureManager->GetErrorTextureHandle();
		}

		if (ai_material->Get(AI_MATKEY_COLOR_DIFFUSE, *(aiColor4D*)&material.baseColor) != AI_SUCCESS) {
			material.baseColor = { 1,1,1,1 };
		}

		if (ai_material->Get(AI_MATKEY_SHININESS, material.roughness) != AI_SUCCESS) {
			material.roughness = 1.0f;
		}

		if (ai_material->Get(AI_MATKEY_REFLECTIVITY, material.metallic) != AI_SUCCESS) {
			material.metallic = 1.0f;
		}

		materials.push_back(material);
	}

	return materials;
}

std::vector<uint32_t> ModelLoader::LoadMaterialIndices(const aiMesh* ai_mesh) {
	std::vector<uint32_t> result;

	for (uint32_t f = 0; f < ai_mesh->mNumFaces; ++f) {
		for (uint32_t i = 0; i < ai_mesh->mFaces[f].mNumIndices; ++i) {
			result.push_back(ai_mesh->mMaterialIndex);
		}
	}

	return result;
}

Skeleton ModelLoader::CreateSkeleton(std::vector<Node>& nodes, const aiScene* scene) {
	Skeleton skeleton{};
	std::unordered_map<std::string, int> boneNameToIndex;

	if (!scene->HasSkeletons()) {
		return skeleton;
	}

	for (int i = 0; i < nodes.size(); ++i) {
		const Node& node = nodes[i];
		if (node.meshIndex != -1) {
			aiMesh* mesh = scene->mMeshes[node.meshIndex];
			for (uint32_t boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex) {
				aiBone* bone = mesh->mBones[boneIndex];
				std::string boneName = bone->mName.C_Str();

				const auto& it = boneNameToIndex.find(boneName);
				if (it == boneNameToIndex.end()) {
					boneNameToIndex[boneName] = static_cast<int>(skeleton.joints.size());
					skeleton.joints.push_back(Joint{});
				}

				auto& joint = skeleton.joints[boneNameToIndex[boneName]];
				joint.name = boneName;

				aiMatrix4x4 bindPoseMatrixAssimp = bone->mOffsetMatrix.Inverse();
				aiVector3D scale, translate;
				aiQuaternion rotate;
				bindPoseMatrixAssimp.Decompose(scale, rotate, translate);

				Matrix4x4 bindPoseMatrix =
					Matrix::MakeScaleMatrix({ scale.x, scale.y, scale.z }) *
					Quaternion(rotate.x, rotate.y, rotate.z, rotate.w).ToMatrix() *
					Matrix::MakeTranslationMatrix({ translate.x, translate.y, translate.z });

				joint.inverseBindMatrix = bindPoseMatrix.Inverse();

				joint.nodeIndex = i;
			}
		}
	}

	return skeleton;
}

std::unordered_map<std::string, Animation> ModelLoader::LoadAnimations(const aiScene* scene) {
	std::unordered_map<std::string, Animation> animations{};

	if (scene->mNumAnimations == 0) {
		return animations;
	}

	for (uint32_t i = 0; i < scene->mNumAnimations; ++i) {
		aiAnimation* ai_animation = scene->mAnimations[i];
		Animation& animation = animations[ai_animation->mName.C_Str()];

		animation.duration = static_cast<float>(ai_animation->mDuration / ai_animation->mTicksPerSecond);

		for (uint32_t channelIndex = 0; channelIndex < ai_animation->mNumChannels; ++channelIndex) {
			aiNodeAnim* ai_nodeAnim = ai_animation->mChannels[channelIndex];
			NodeAnimation nodeAnimation{};
			//位置
			for (uint32_t posIndex = 0; posIndex < ai_nodeAnim->mNumPositionKeys; ++posIndex) {
				KeyframeVector3 keyframe{};
				keyframe.time = static_cast<float>(ai_nodeAnim->mPositionKeys[posIndex].mTime / ai_animation->mTicksPerSecond);
				keyframe.value.x = ai_nodeAnim->mPositionKeys[posIndex].mValue.x;
				keyframe.value.y = ai_nodeAnim->mPositionKeys[posIndex].mValue.y;
				keyframe.value.z = ai_nodeAnim->mPositionKeys[posIndex].mValue.z;
				nodeAnimation.position.keyframes.push_back(keyframe);
			}
			//回転
			for (uint32_t rotIndex = 0; rotIndex < ai_nodeAnim->mNumRotationKeys; ++rotIndex) {
				KeyframeQuaternion keyframe{};
				keyframe.time = static_cast<float>(ai_nodeAnim->mRotationKeys[rotIndex].mTime / ai_animation->mTicksPerSecond);
				keyframe.value.x = ai_nodeAnim->mRotationKeys[rotIndex].mValue.x;
				keyframe.value.y = ai_nodeAnim->mRotationKeys[rotIndex].mValue.y;
				keyframe.value.z = ai_nodeAnim->mRotationKeys[rotIndex].mValue.z;
				keyframe.value.w = ai_nodeAnim->mRotationKeys[rotIndex].mValue.w;
				nodeAnimation.rotate.keyframes.push_back(keyframe);
			}
			//スケール
			for (uint32_t scaleIndex = 0; scaleIndex < ai_nodeAnim->mNumScalingKeys; ++scaleIndex) {
				KeyframeVector3 keyframe{};
				keyframe.time = static_cast<float>(ai_nodeAnim->mScalingKeys[scaleIndex].mTime / ai_animation->mTicksPerSecond);
				keyframe.value.x = ai_nodeAnim->mScalingKeys[scaleIndex].mValue.x;
				keyframe.value.y = ai_nodeAnim->mScalingKeys[scaleIndex].mValue.y;
				keyframe.value.z = ai_nodeAnim->mScalingKeys[scaleIndex].mValue.z;
				nodeAnimation.scale.keyframes.push_back(keyframe);
			}

			animation.nodeAnimations[ai_nodeAnim->mNodeName.C_Str()] = nodeAnimation;
		}

	}

	return animations;
}
