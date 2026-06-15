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

		double GetTimeStampResult(std::string name);

	private:

		struct TimeStamp {
			std::string name;

			int startHandle = -1;
			int endHandle = -1;

			double time = 0.0;

			std::vector<std::unique_ptr<TimeStamp>> children{};
			TimeStamp* parent = nullptr;
		};

		TimeStamp* FindTimeStamp(const std::string& name, TimeStamp* current = nullptr);

		void PutTimeStamp(CmdObj* cmdObj, int handle);

		std::unique_ptr<TimeStamp> root_;
		TimeStamp* current_ = nullptr;

		int nextHandle_ = 0;
		const int queryCount_ = 256;
		uint64_t frequency_ = 0;

		template <typename T>
		struct Com {
			Microsoft::WRL::ComPtr<T> ptr;
		};

		std::vector<Com<ID3D12QueryHeap>> queryHeap_;

		std::vector<Com<ID3D12Resource>> readBackBuffers_;
		std::vector<uint64_t*> mappedBuffers_;
		std::vector<uint64_t> timeStampResults_;

		Logger logger_;

	};

}
