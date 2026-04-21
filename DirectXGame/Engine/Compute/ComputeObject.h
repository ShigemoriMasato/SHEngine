#pragma once
#include <Tool/Logger/Logger.h>
#include <Core/Command/CommandManager.h>
#include <Compute/PSO/CSPSOManager.h>
#include <Render/Buffer/BufferContainer.h>

namespace SHEngine {

	class ComputeObject {
	public:

		ComputeObject(std::string debugName = "");
		~ComputeObject() = default;

		static void StaticInitialize(PSO::CSPSOManager* psoManager) { psoManager_ = psoManager; };
		void Initialize();

		// @brief Shader名を登録
		void SetShader(std::string computeShaderName) { computeShaderName_ = computeShaderName; }
		// @brief スレッドグループのサイズを登録
		void SetThreadGroupSize(int x, int y = 1, int z = 1) { threadGroupSize_ = { x, y, z }; }
		// @brief CBV/SRV/UAVを登録
		void SetGPUBuffer(BufferType bufferType, GPUBuffer* buffer);
		// @brief 複数のCBV/SRV/UAVを登録
		void SetGPUBuffers(BufferType bufferType, std::vector<GPUBuffer*> buffers);

		// @brief 登録したCBV/SRV/UAVをComputeShaderにセットして、ComputeShaderを実行する。
		// @param cmdObj コマンドオブジェクト。この関数の後にengine_->ExecuteCommandを呼び出すこと。
		void Execute(CmdObj* cmdObj);

	private:

		static PSO::CSPSOManager* psoManager_;

		struct BufferData {
			void* mapped = nullptr;
			size_t size = 0;
		};

		std::map<BufferType, std::vector<GPUBuffer*>> gpuBuffers_;
		std::string computeShaderName_;

		struct ThreadGroupSize {
			int x;
			int y;
			int z;
		} threadGroupSize_;

		Logger logger_ = getLogger("Engine");
		std::string debugName_;
	};

}
