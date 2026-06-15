#pragma once
#include <Core/DXDevice.h>
#include <Core/Command/CommandObject.h>

namespace SHEngine {

	class MeasureShaderTime {
	public:

		void Initialize(DXDevice* device, ID3D12CommandQueue* queue);

		void NewFrame(CmdObj* cmdObj);

		void FinFrame(CmdObj* cmdObj);

		void Begin(CmdObj* cmdObj, std::string name);

		void End(CmdObj* cmdObj);

		double GetTimeStampResult(int index);

	private:

		void CreateTimeStamp(CmdObj* cmdObj, std::string name);

		int PutTimeStamp(CmdObj* cmdObj, int handle);

		struct TimeStamp {
			std::string name;

			int startHandle;
			int endHandle;

			double time = 0.0;

			std::vector<std::unique_ptr<TimeStamp>> children;
		};

		TimeStamp root_;

		const int queryCount_ = 256;
		uint64_t frequency_ = 0;

		template <typename T>
		struct Com {
			Microsoft::WRL::ComPtr<T> ptr;
		};

		std::vector<Com<ID3D12QueryHeap>> queryHeap_;

		std::vector<Com<ID3D12Resource>> readBackBuffers_;
		std::vector<uint64_t*> mappedBuffers_;
		std::vector<std::vector<double>> timeStampResults_;

		Logger logger_;

	};

}
