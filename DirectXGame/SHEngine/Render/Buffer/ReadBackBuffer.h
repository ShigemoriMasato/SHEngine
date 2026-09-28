#pragma once
#include <Core/Command/ICommandContext.h>
#include <Render/Buffer/GPUBuffer.h>

namespace SHEngine {

	class ReadBackBuffer {
	public:

		static void SetDevice(DXDevice* device) { device_ = device; }
		ReadBackBuffer(size_t size);

		// Readbackに対してデータをコピーするコマンドを積む
		void Copy(SHEngine::GPUBuffer* src, SHEngine::ICommandContext* context);

		void* GetData(SHEngine::ICommandContext* context) { return data_[context->GetCurrentID()]; }

	private:

		static inline DXDevice* device_ = nullptr;

		struct Resource {
			Microsoft::WRL::ComPtr<ID3D12Resource> res;
		};
		std::vector<Resource> resources_ = {};
		std::vector<void*> data_ = {};

	};
}