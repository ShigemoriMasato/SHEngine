#include "MeasureShaderTime.h"
#include <Utility/DirectUtilFuncs.h>

void SHEngine::MeasureShaderTime::Initialize(DXDevice* device, ID3D12CommandQueue* queue) {
	int bufferCount = device->GetBufferCount();

	logger_ = GetLogger("Engine");

	for (int i = 0; i < bufferCount; ++i) {
		D3D12_QUERY_HEAP_DESC desc{};
		desc.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
		desc.Count = queryCount_;

		device->GetDevice()->CreateQueryHeap(&desc, IID_PPV_ARGS(&queryHeap_[i].ptr));

		auto readBack = Func::CreateReadBackResource(device->GetDevice(), sizeof(uint64_t) * queryCount_);
		readBackBuffers_[i].ptr.Attach(readBack);
	}

	queue->GetTimestampFrequency(&frequency_);

	timeStampResults_.resize(bufferCount);
	for (auto& vec : timeStampResults_) {
		vec.resize(queryCount_ - 1);
	}

	readBackBuffers_.resize(bufferCount);
	for (int i = 0; i < bufferCount; ++i) {
		uint64_t* mappedData = nullptr;
		readBackBuffers_[i].ptr->Map(0, nullptr, reinterpret_cast<void**>(&mappedData));
		mappedBuffers_[i] = mappedData;
	}
}

void SHEngine::MeasureShaderTime::NewFrame(CmdObj* cmdObj) {
	int frameIndex = cmdObj->GetCurrentID();

	auto& rowData = mappedBuffers_[frameIndex];

	// クエリヒープとリードバックバッファをマップして、タイムスタンプの結果を取得
	auto& heap = queryHeap_[frameIndex].ptr;
	auto& buffer = readBackBuffers_[frameIndex].ptr;
	
	for (int i = 0; i < queryCount_ - 1; ++i) {
		uint64_t timeStamp = rowData[i + 1] - rowData[i];
		timeStampResults_[frameIndex][i] = static_cast<double>(timeStamp) / frequency_;
	}

	root_.children.clear();
}

void SHEngine::MeasureShaderTime::FinFrame(CmdObj* cmdObj) {
	auto cmdList = cmdObj->GetCommandList();

	// クエリヒープからリードバックバッファにタイムスタンプの結果をコピー
	cmdList->ResolveQueryData(queryHeap_[cmdObj->GetCurrentID()].ptr.Get(), D3D12_QUERY_TYPE_TIMESTAMP,
		0, queryCount_, readBackBuffers_[cmdObj->GetCurrentID()].ptr.Get(), 0);
}

void SHEngine::MeasureShaderTime::Begin(CmdObj* cmdObj, std::string name) {

}

void SHEngine::MeasureShaderTime::End(CmdObj* cmdObj) {
}

double SHEngine::MeasureShaderTime::GetTimeStampResult(int index) {
	// 0xffffffffの場合は、クエリの上限に達している場合がある。その場合は既にLogを残しているので、わかりやすいように-999.9を返す
	if (index == 0xffffffff) {
		return -999.9;
	}

	int id = index >> 24;
	index = index & 0xFFFFFF;

	//それ以外のエラーの場合は、ログを残して、assertで止める
	if (index < 0 || index >= queryCount_ - 1 || id >= int(timeStampResults_.size()) || id < 0) {
		logger_->error("タイムスタンプのインデックスが範囲外です。");
		assert(false && "MeasureShaderTime 謎のインデックスを読み込みました");
		return 65535.0;
	}

	return timeStampResults_[id][index];
}

void SHEngine::MeasureShaderTime::CreateTimeStamp(CmdObj* cmdObj, std::string name) {


}

int SHEngine::MeasureShaderTime::PutTimeStamp(CmdObj* cmdObj, int handle) {
	if (handle >= queryCount_) {
		assert(false && "MeasureShaderTime クエリの上限に達しています");
		return 0xffffffff;
	}

	auto cmdList = cmdObj->GetCommandList();
	int id = cmdObj->GetCurrentID() << 24;

	// タイムスタンプをクエリヒープに書き込む
	cmdList->EndQuery(queryHeap_[cmdObj->GetCurrentID()].ptr.Get(), D3D12_QUERY_TYPE_TIMESTAMP, handle);
}
