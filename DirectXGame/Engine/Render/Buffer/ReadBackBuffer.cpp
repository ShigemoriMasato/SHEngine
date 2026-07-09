#include "ReadBackBuffer.h"
#include <Utility/DirectUtilFuncs.h>

SHEngine::ReadBackBuffer::ReadBackBuffer(size_t size) {
	int bufferCount = device_->GetBufferCount();

	for (int i = 0; i < bufferCount; ++i) {
		auto& res = resources_.emplace_back();
		res.res.Attach(SHEngine::Func::CreateReadBackResource(device_->GetDevice(), size));

		auto& mappedData = data_.emplace_back();
		res.res->Map(0, nullptr, &mappedData);
	}
}

void SHEngine::ReadBackBuffer::Copy(SHEngine::GPUBuffer* src, SHEngine::ICommandContext* context) {
	auto cmdList = context->GetCommandList();
	int id = context->GetCurrentID();

	//Barrierの張り替え (ReadBackは常にCopy_Dest)
	src->TransitionBarrier(D3D12_RESOURCE_STATE_COPY_SOURCE);
	src->Flush(cmdList);

	//Copyコマンドを積む
	cmdList->CopyResource(resources_[id].res.Get(), src->GetResource());
}
