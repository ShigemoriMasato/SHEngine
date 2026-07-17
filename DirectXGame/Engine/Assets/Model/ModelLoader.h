#pragma once
#include "ModelData.h"
#include <assimp/scene.h>
#include <unordered_set>
#include <Assets/Texture/TextureManager.h>

namespace ModelLoader {
	void CreateNodes(const aiNode* node, std::vector<Node>& nodes, uint32_t parentIndex);

	std::vector<Node> LoadNodes(const aiScene* scene);
	std::vector<Mesh> LoadMeshes(const aiScene* scene);
	std::vector<Vector4> LoadPositions(const aiMesh* ai_mesh);
	std::vector<Vector3> LoadNormals(const aiMesh* ai_mesh);
	std::vector<Vector2> LoadTexcoords(const aiMesh* ai_mesh);
	std::vector<Vector4> LoadColors(const aiMesh* ai_mesh);
	std::vector<VertexInfluence> LoadVertexInfluences(const aiMesh* ai_mesh);
	std::vector<uint32_t> LoadIndices(const aiMesh* ai_mesh);

	std::vector<Material> LoadMaterials(const aiScene* scene, std::string directoryPath, SHEngine::TextureManager* textureManager);
	std::vector<uint32_t> LoadMaterialIndices(const aiMesh* ai_mesh);

	Skeleton CreateSkeleton(std::vector<Node>& nodes, const aiScene* scene);

	std::unordered_map<std::string, Animation>  LoadAnimations(const aiScene* scene);
}
