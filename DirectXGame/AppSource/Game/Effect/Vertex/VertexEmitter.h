#pragma once
#include <Game/Effect/Base/IEmitter.h>
#include <Render/Buffer/ReadBackBuffer.h>

class VertexEmitter : public IEmitter {
public:

	void Initialize(SHEngine::Engine* engine, const Pool& pool) override;

	int AddModel(const std::vector<Vector3>& vertices, const Vector4& color, CCC* ccc);
	void ReleaseModel(int index, CCC* ccc);
	void EditColor(int index, const Vector4& color, CCC* ccc);
	void EditPosition(int index, const std::vector<Vector3>& vertices, CCC* ccc);

	void CopyIndexList(int index, SHEngine::ReadBackBuffer* dest, CCC* ccc);

	void Update(CCC* ccc, float deltaTime) override;

private:

	void SizeCheck(int index);

	struct Data {
		SHEngine::GPUBuffer* indexList = nullptr;			//そのモデルが占有しているインデックスのリスト
		SHEngine::GPUBuffer* vertexList = nullptr;			//モデルの頂点情報
		SHEngine::GPUBuffer* color = nullptr;				//そのモデルの色情報
		SHEngine::GPUBuffer* vertexNumBuffer = nullptr;		//そのモデルの頂点数を格納するバッファ
		uint32_t vertexNum = 0;								//頂点数。Dispatchに使用
	};

	std::unique_ptr<SHEngine::ComputeObject> addModel_ = nullptr;
	std::unique_ptr<SHEngine::ComputeObject> editVertex_ = nullptr;
	std::unique_ptr<SHEngine::ComputeObject> editColor_ = nullptr;
	std::unique_ptr<SHEngine::ComputeObject> release_ = nullptr;
	std::unique_ptr<SHEngine::ComputeObject> copyBuffer_ = nullptr;

	SHEngine::GPUBuffer* positions_ = nullptr;
	SHEngine::GPUBuffer* colors_ = nullptr;
	SHEngine::GPUBuffer* freeList_ = nullptr;
	SHEngine::GPUBuffer* freeListIndex_ = nullptr;

	//スワップチェーン分のバッファ全てにデータをコピーするためのリスト。firstが何個コピーしたか。
	std::vector<std::pair<int, Data*>> needUpdate_ = {};
	std::vector<std::unique_ptr<Data>> modelData_ = {};

	uint32_t nextID_ = 0;

};
