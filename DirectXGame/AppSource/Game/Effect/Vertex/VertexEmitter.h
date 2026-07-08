#pragma once
#include <Game/Effect/Base/IEmitter.h>

class VertexEmitter : public IEmitter {
public:

	void Initialize(SHEngine::Engine* engine, const Pool& pool) override;

	int AddModel(const std::vector<Vector3>& vertices, CCC* ccc);
	void EraseModel(int index, CCC* ccc);
	void EditColor(int index, const Vector3& color, CCC* ccc);
	void EditPosition(int index, const std::vector<Vector3>& vertices, CCC* ccc);

	void Update(CCC* ccc, float deltaTime) override;

private:

	struct Data {
		SHEngine::GPUBuffer* indexList = nullptr;			//そのモデルが占有しているインデックスのリスト
		SHEngine::GPUBuffer* vertexList = nullptr;			//モデルの頂点情報
		SHEngine::GPUBuffer* color = nullptr;				//そのモデルの色情報
		SHEngine::GPUBuffer* vertexNumBuffer = nullptr;		//そのモデルの頂点数を格納するバッファ
		uint32_t vertexNum = 0;								//頂点数。Dispatchに使用
	};

	std::unique_ptr<SHEngine::ComputeObject> addModel_ = nullptr;
	std::unique_ptr<SHEngine::ComputeObject> editPosition_ = nullptr;
	std::unique_ptr<SHEngine::ComputeObject> editColor_ = nullptr;
	std::unique_ptr<SHEngine::ComputeObject> release_ = nullptr;

	SHEngine::GPUBuffer* positions_ = nullptr;
	SHEngine::GPUBuffer* colors_ = nullptr;
	SHEngine::GPUBuffer* freeList_ = nullptr;
	SHEngine::GPUBuffer* freeListIndex_ = nullptr;

	std::vector<Data> modelData_ = {};

	uint32_t nextID_ = 0;

};
