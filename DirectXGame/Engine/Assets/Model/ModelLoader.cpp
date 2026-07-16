#include "ModelLoader.h"
#include <Utility/MatrixFactory.h>
#include <Utility/MyMath.h>

using namespace SHEngine;

void ModelLoader::CreateNodes(const aiNode* node, std::vector<Node>& nodes, uint32_t parentIndex) {
	uint32_t nodeIndex = static_cast<uint32_t>(nodes.size());
	Node newNode = nodes.emplace_back();
	
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

std::vector<Vector4> ModelLoader::LoadPositions(const aiMesh* ai_mesh) {
	std::vector<Vector4> positions;

	positions.resize(ai_mesh->mNumVertices);
	for (uint32_t v = 0; v < ai_mesh->mNumVertices; ++v) {
		//位置
		positions[v].x = ai_mesh->mVertices[v].x;
		positions[v].y = ai_mesh->mVertices[v].y;
		positions[v].z = ai_mesh->mVertices[v].z;
		positions[v].w = 1.0f;
	}

	return positions;
}

std::vector<Vector3> ModelLoader::LoadNormals(const aiMesh* ai_mesh) {
	std::vector<Vector3> normals;

	normals.resize(ai_mesh->mNumVertices);
	for (uint32_t v = 0; v < ai_mesh->mNumVertices; ++v) {
		//法線
		normals[v].x = ai_mesh->mNormals[v].x;
		normals[v].y = ai_mesh->mNormals[v].y;
		normals[v].z = ai_mesh->mNormals[v].z;
	}

	return normals;
}

std::vector<Vector2> ModelLoader::LoadTexcoords(const aiMesh* ai_mesh) {
	std::vector<Vector2> texcoords;

	texcoords.resize(ai_mesh->mNumVertices);
	for (uint32_t v = 0; v < ai_mesh->mNumVertices; ++v) {
		//UV
		texcoords[v].x = ai_mesh->mTextureCoords[0][v].x;
		texcoords[v].y = ai_mesh->mTextureCoords[0][v].y;
	}

	return texcoords;
}

std::vector<Vector4> ModelLoader::LoadColors(const aiMesh* ai_mesh) {
	std::vector<Vector4> colors;

	colors.resize(ai_mesh->mNumVertices);
	for (uint32_t v = 0; v < ai_mesh->mNumVertices; ++v) {
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
	vertexInfluences.resize(ai_mesh->mNumVertices);

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
	return std::vector<uint32_t>();
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
		} else {
			material.textureIndex = textureManager->GetWhite1x1Texture();
		}



		materials.push_back(material);
	}

	return materials;
}

std::vector<uint32_t> ModelLoader::LoadMaterialIndices(const aiScene* scene) {
	std::vector<uint32_t> result;

	for (uint32_t mesh = 0; mesh < scene->mNumMeshes; ++mesh) {
		aiMesh* ai_mesh = scene->mMeshes[mesh];

		for (uint32_t f = 0; f < ai_mesh->mNumFaces; ++f) {
			for (uint32_t i = 0; i < ai_mesh->mFaces[f].mNumIndices; ++i) {
				result.push_back(ai_mesh->mMaterialIndex);
			}
		}
	}

	return result;
}

std::map<std::string, Skin> ModelLoader::LoadSkinCluster(const aiScene* scene) {
	std::map<std::string, Skin> skinClusterData;

	for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {
		aiMesh* mesh = scene->mMeshes[meshIndex];

		for (uint32_t boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex) {
			aiBone* bone = mesh->mBones[boneIndex];
			std::string jointName = bone->mName.C_Str();
			Skin& jointWeightData = skinClusterData[jointName];

			aiMatrix4x4 bindPoseMatrixAssimp = bone->mOffsetMatrix.Inverse();
			aiVector3D scale, translate;
			aiQuaternion rotate;
			bindPoseMatrixAssimp.Decompose(scale, rotate, translate);
			Matrix4x4 bindPoseMatrix =
				Matrix::MakeScaleMatrix({ scale.x, scale.y, scale.z }) *
				Quaternion(rotate.x, rotate.y, rotate.z, rotate.w).ToMatrix() *
				Matrix::MakeTranslationMatrix({ translate.x, translate.y, translate.z });
			jointWeightData.inverseBindPoseMatrix = bindPoseMatrix.Inverse();

			for (uint32_t weightIndex = 0; weightIndex < bone->mNumWeights; ++weightIndex) {
				jointWeightData.vertexWeights.push_back({ bone->mWeights[weightIndex].mWeight, bone->mWeights[weightIndex].mVertexId });
			}
		}
	}

	return skinClusterData;
}

Skeleton ModelLoader::CreateSkelton(const Node& rootNode, const aiScene* scene) {
	Skeleton skeleton{};
	std::unordered_set<std::string> boneNames;

	for (uint32_t i = 0; i < scene->mNumMeshes; ++i) {
		for (uint32_t b = 0; b < scene->mMeshes[i]->mNumBones; ++b) {
			boneNames.insert(scene->mMeshes[i]->mBones[b]->mName.C_Str());
		}
	}
	skeleton.root = CreateJoint(rootNode, {}, skeleton, boneNames, Matrix4x4::Identity());

	//名前とindexのマッピング
	for (const Joint& joint : skeleton.joints) {
		skeleton.jointMap[joint.name] = joint.index;
	}

	return skeleton;
}

int32_t ModelLoader::CreateJoint(const Node& node, const std::optional<int32_t>& parent, Skeleton& skeleton, std::unordered_set<std::string>& boneNames, Matrix4x4 parentAccumulated) {
	Matrix4x4 currentAccumulated = parentAccumulated * node.localMatrix;

	auto& joints = skeleton.joints;
	if (!boneNames.contains(node.name)) {
		// Jointにしないが、子は見る
		for (const Node& child : node.children) {
			skeleton.rootMatrix = node.localMatrix * skeleton.rootMatrix;
			CreateJoint(child, parent, skeleton, boneNames, currentAccumulated);
		}
		return -1;
	}

	Joint joint{};
	joint.name = node.name;
	joint.localMatrix = node.localMatrix;
	joint.skeletonSpaceMatrix = Matrix4x4::Identity();
	joint.transform = node.transform;
	joint.index = static_cast<int32_t>(joints.size());
	joint.parent = parent;

	joints.push_back(joint);

	if (!parent.has_value()) {
		skeleton.rootMatrix = parentAccumulated;
	}

	for (const Node& child : node.children) {
		int32_t childIndex = CreateJoint(child, joint.index, skeleton, boneNames, Matrix4x4::Identity());
		joints[joint.index].children.push_back(childIndex);
	}

	return joint.index;
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

PolygonList ModelLoader::LoadPolygonList(const aiScene* scene) {
	PolygonList polygonList{};

	auto getVertexPosition = [](const aiMesh* mesh, uint32_t vertexIndex) -> Vector3 {
		return { mesh->mVertices[vertexIndex].x, mesh->mVertices[vertexIndex].y, mesh->mVertices[vertexIndex].z };
		};

	auto getArea = [](Vector3 a, Vector3 b, Vector3 c) -> float {
		Vector3 ab = b - a;
		Vector3 ac = c - a;
		Vector3 cross = MyMath::cross(ab, ac);
		return 0.5f * cross.Length();
		};

	for (uint32_t mesh = 0; mesh < scene->mNumMeshes; ++mesh) {
		aiMesh* ai_mesh = scene->mMeshes[mesh];

		//メモリ確保
		polygonList.polygons.reserve(polygonList.polygons.size() + ai_mesh->mNumFaces);
		polygonList.areas.reserve(polygonList.areas.size() + ai_mesh->mNumFaces);

		for (uint32_t face = 0; face < ai_mesh->mNumFaces; ++face) {
			aiFace* ai_face = &ai_mesh->mFaces[face];

			//三角面化されていない場合はエラー
			if (ai_face->mNumIndices != 3) {
				throw std::runtime_error("Not a Triangle Face!!");
			}

			//頂点と面積を入力
			PolygonData& polygon = polygonList.polygons.emplace_back();
			polygon.a = getVertexPosition(ai_mesh, ai_face->mIndices[0]);
			polygon.b = getVertexPosition(ai_mesh, ai_face->mIndices[1]);
			polygon.c = getVertexPosition(ai_mesh, ai_face->mIndices[2]);

			float& area = polygonList.areas.emplace_back();
			area = getArea(polygon.a, polygon.b, polygon.c);

			polygonList.totalArea += area;
		}
	}

	return polygonList;
}
