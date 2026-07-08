#include "MeasureShaderTime.h"
#include <Utility/DirectUtilFuncs.h>
#include <Core/Command/ICommandContext.h>

void SHEngine::MeasureShaderTime::Initialize(DXDevice* device, ID3D12CommandQueue* queue) {
	int bufferCount = device->GetBufferCount();

	logger_ = GetLogger("Engine");

	queryHeap_.resize(bufferCount);
	readBackBuffers_.resize(bufferCount);
	for (int i = 0; i < bufferCount; ++i) {
		D3D12_QUERY_HEAP_DESC desc{};
		desc.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
		desc.Count = queryCount_;

		device->GetDevice()->CreateQueryHeap(&desc, IID_PPV_ARGS(&queryHeap_[i].ptr));

		auto readBack = Func::CreateReadBackResource(device->GetDevice(), sizeof(uint64_t) * queryCount_);
		readBackBuffers_[i].ptr.Attach(readBack);
	}

	queue->GetTimestampFrequency(&frequency_);

	timeStampResults_.resize(queryCount_ - 1);

	readBackBuffers_.resize(bufferCount);
	mappedBuffers_.resize(bufferCount);
	for (int i = 0; i < bufferCount; ++i) {
		uint64_t* mappedData = nullptr;
		readBackBuffers_[i].ptr->Map(0, nullptr, reinterpret_cast<void**>(&mappedData));
		mappedBuffers_[i] = mappedData;
	}

	root_ = std::make_unique<TimeStamp>();
}

void SHEngine::MeasureShaderTime::NewFrame(ICommandContext* commandContext) {
	int frameIndex = commandContext->GetCurrentID();

	auto rowData = mappedBuffers_[frameIndex];

	for (int i = 0; i < queryCount_ - 1; ++i) {
		timeStampResults_[i] = rowData[i];
	}

	root_->children.clear();
	nextHandle_ = 0;

	current_ = nullptr;

	Begin(commandContext, "Default-GPUTime");
}

void SHEngine::MeasureShaderTime::FinFrame(ICommandContext* commandContext) {
	End(commandContext);

	auto cmdList = commandContext->GetCommandList();

	if (root_->endHandle == -1) {
		logger_->error("MeasureShaderTime: endHandleが異常値でした。");
		assert(false && "MeasureShaderTime: endHandleが異常値でした。");
		return;
	}

	// クエリヒープからリードバックバッファにタイムスタンプの結果をコピー
	cmdList->ResolveQueryData(queryHeap_[commandContext->GetCurrentID()].ptr.Get(), D3D12_QUERY_TYPE_TIMESTAMP,
		0, root_->endHandle, readBackBuffers_[commandContext->GetCurrentID()].ptr.Get(), 0);
}

void SHEngine::MeasureShaderTime::Begin(ICommandContext* commandContext, std::string name) {
	auto timeStamp = std::make_unique<TimeStamp>();
	timeStamp->name = name;
	timeStamp->startHandle = nextHandle_++;
	timeStamp->parent = current_;

	// タイムスタンプをクエリヒープに書き込む
	PutTimeStamp(commandContext, timeStamp->startHandle);
	
	//currentの設定
	if (current_) {
		current_->children.push_back(std::move(timeStamp));
		current_ = current_->children.back().get();
	} else {
		root_ = std::move(timeStamp);
		current_ = root_.get();
	}
}

void SHEngine::MeasureShaderTime::End(ICommandContext* commandContext) {
	// currentがnullptrのときは、Beginが呼ばれていないのにEndが呼ばれたことになるので、assertで止める
	if (!current_) {
		logger_->error("MeasureShaderTime::Endが呼ばれましたが、Beginが呼ばれていません。");
		assert(false && "MeasureShaderTime::Endが呼ばれましたが、Beginが呼ばれていません。");
		return;
	}

	// Handleを割り当ててヒープに書き込む
	current_->endHandle = nextHandle_++;
	PutTimeStamp(commandContext, current_->endHandle);

	// 階層を一つ下げる
	current_ = current_->parent;
}

double SHEngine::MeasureShaderTime::GetTimeStampResult(std::string name) {
	auto timeStamp = FindTimeStamp(name);
	if (!timeStamp) {
		logger_->error("タイムスタンプ '{}' が見つかりません。", name);
		return 0.0;
	}

	return static_cast<double>(timeStampResults_[timeStamp->endHandle] - timeStampResults_[timeStamp->startHandle]) / frequency_;
}

SHEngine::MeasureShaderTime::TimeStamp* SHEngine::MeasureShaderTime::FindTimeStamp(const std::string& name, TimeStamp* current) {
	if (!current) {
		current = root_.get();
	}

	if (current->name == name) {
		return current;
	}

	for (const auto& child : current->children) {
		auto result = FindTimeStamp(name, child.get());
		if (result) {
			return result;
		}
	}

	return nullptr;
}

void SHEngine::MeasureShaderTime::PutTimeStamp(ICommandContext* commandContext, int handle) {
	if (handle >= queryCount_) {
		assert(false && "MeasureShaderTime クエリの上限に達しています");
		return;
	}

	auto cmdList = commandContext->GetCommandList();
	int id = commandContext->GetCurrentID() << 24;

	// タイムスタンプをクエリヒープに書き込む
	cmdList->EndQuery(queryHeap_[commandContext->GetCurrentID()].ptr.Get(), D3D12_QUERY_TYPE_TIMESTAMP, handle);
}
