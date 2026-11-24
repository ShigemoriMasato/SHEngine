#include "DeathEffect.h"
#include <Utility/MyMath.h>
#include <Utility/SearchFile.h>

DeathEffect::DeathEffect(ParticleSystem* particleSystem, SHEngine::ModelManager* modelManager, ICC* icc) {
	std::vector<Vector3> vertices;
	std::vector<uint32_t> indices;
	std::vector<int> chanceList;
	std::vector<int> chanceListBuffer;

	// GameObjectのモデル名をすべて取得
	auto modelNames = SearchDirectoryPathsAddChild("Assets/Model/GameObject");

	container_ = std::make_unique<SHEngine::BufferContainer>();

	for (const auto& modelName : modelNames) {
		vertices.clear();
		indices.clear();
		std::vector<float> areas;
		chanceList.clear();
		chanceListBuffer.clear();
		
		auto modelData = modelManager->GetModelData(modelName);
		if (!modelData) {
			continue;
		}

		//面積を求める
		float minArea = 0.0f;
		for (int i = 0; i < modelData->meshes.size(); ++i) {
			const auto& mesh = modelData->meshes[i];
			vertices.insert(vertices.end(), mesh.position.begin(), mesh.position.end());
			indices.insert(indices.end(), mesh.indices.begin(), mesh.indices.end());
			
			areas.resize(mesh.indices.size() / 3);
			for (int j = 0; j < (int)mesh.indices.size() / 3; ++j) {
				Vector3 a, b, c;
				a = mesh.position[mesh.indices[j * 3 + 0]];
				b = mesh.position[mesh.indices[j * 3 + 1]];
				c = mesh.position[mesh.indices[j * 3 + 2]];

				float area = 0.5f * (MyMath::cross((b - a), (c - a))).Length();
				minArea = std::min(minArea, area);
				areas[i] = area;
			}
		}

		// 一番小さい面積がスロット4つ分として、スロット数と面積の大きさを比例させる。
		int chanceListSize = 0;
		for (const auto& area : areas) {
			int slotCount = (int)(std::roundf(area / minArea)) * 4;
			chanceListSize += slotCount;

			chanceListBuffer.push_back(slotCount);
		}

		// chanceListを作る
		chanceList.resize(chanceListSize);
		for (int i = 0; i < (int)areas.size(); ++i) {
			chanceList.insert(chanceList.end(), chanceListBuffer[i], i);
		}

		auto& polygon = polygonDataMap_[modelName];
		polygon.vertex = container_->Create(BufferType::SRV, sizeof(Vector3), (uint32_t)vertices.size(), BufferNum::Single);
		polygon.index = container_->Create(BufferType::SRV, sizeof(uint32_t), (uint32_t)indices.size(), BufferNum::Single);
		polygon.chanceList = container_->Create(BufferType::SRV, sizeof(int), (uint32_t)areas.size(), BufferNum::Single);
		polygon.chanceListSize = container_->Create(BufferType::CBV, sizeof(int), 1, BufferNum::Single);

		polygon.vertex->CopyBuffer(vertices.data(), sizeof(Vector3) * vertices.size());
		polygon.index->CopyBuffer(indices.data(), sizeof(uint32_t) * indices.size());
		polygon.chanceList->CopyBuffer(chanceList.data(), sizeof(int) * chanceList.size());
		polygon.chanceListSize->CopyBuffer(&chanceListSize, sizeof(int));
	}

	emitter_ = particleSystem->CreateEmitter(maxEmitCount_, icc);

	
}

void DeathEffect::Initialize() {

}
