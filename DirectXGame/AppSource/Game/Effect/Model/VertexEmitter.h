#pragma once
#include <Game/Effect/Base/IEmitter.h>

class VertexEmitter : public IEmitter {
public:

	VertexEmitter();

	void Initialize(SHEngine::Engine* engine, const Pool& pool) override;

	int AddModel(const std::vector<Vector3>& vertices);
	void EraseModel(int index);
	void EditColor(int index, const Vector3& color);

	void Update(CmdObj* compute, float deltaTime) override;

private:

	struct Data {
		SHEngine::GPUBuffer* indexList = nullptr;			//そのモデルが占有しているインデックスのリスト
		SHEngine::GPUBuffer* vertexList = nullptr;			//モデルの頂点情報
		SHEngine::GPUBuffer* color = nullptr;				//そのモデルの色情報
		uint32_t vertexNum = 0;								//頂点数。Dispatchに使用
	};

	std::unique_ptr<SHEngine::ComputeObject> update_ = nullptr;

	SHEngine::GPUBuffer* positions_ = nullptr;
	SHEngine::GPUBuffer* colors_ = nullptr;

	std::vector<Data> modelData_ = {};

	uint32_t nextID_ = 0;

};
