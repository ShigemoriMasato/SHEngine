#pragma once
#include <Tool/Logger/Logger.h>
#include <Compute/ComputeCommandContext.h>
#include <Compute/PSO/CSPSOManager.h>
#include <Render/Buffer/BufferContainer.h>

namespace SHEngine {

	class ComputeObject {
	public:

		ComputeObject(std::string debugName = "");
		~ComputeObject() = default;

		static void StaticInitialize(PSO::CSPSOManager* psoManager, D3D12_GPU_DESCRIPTOR_HANDLE textureStartHandle);
		void Initialize();

		// @brief Shader名を登録
		void SetShader(std::string computeShaderName) { computeShaderName_ = computeShaderName; }
		// @brief スレッドグループのサイズを登録
		void SetExecuteNum(int executeNum);
		// @brief CBV/SRV/UAVを登録
		void SetGPUBuffer(BufferType bufferType, GPUBuffer* buffer);
		// @brief 複数のCBV/SRV/UAVを登録
		void SetGPUBuffers(BufferType bufferType, std::vector<GPUBuffer*> buffers);
		// @brief テクスチャを使用するかどうかを登録。ComputeShaderでテクスチャを使用する場合はこれをtrueにする必要がある。使用するレジスタはt8~t1032(最大1024個)。
		void SetUseTexture(bool useTexture);
		// @brief SamplerIDをOR演算子で登録。
		void SetSamplerID(uint32_t samplerID) { samplerID_ = samplerID; }
		void SetSamplerID(SHEngine::PSO::SamplerID samplerID) { samplerID_ = uint32_t(samplerID); }

		// @brief 登録したCBV/SRV/UAVをComputeShaderにセットして、ComputeShaderを実行する。
		// @param commandContext コマンドコンテキスト。DCCでもできるため、CCCである必要はないが、CCC推奨。
		void Execute(SHEngine::ICommandContext* commandContext);

	private:

		static PSO::CSPSOManager* psoManager_;
		static D3D12_GPU_DESCRIPTOR_HANDLE textureStartHandle_;

		struct BufferData {
			void* mapped = nullptr;
			size_t size = 0;
		};

		std::map<BufferType, std::vector<GPUBuffer*>> gpuBuffers_;
		std::string computeShaderName_;
		bool useTexture_ = false;
		uint32_t samplerID_ = 0;
		struct ThreadGroupSize {
			int x;
			int y;
			int z;
		} threadGroupSize_;

		Logger logger_ = GetLogger("Engine");
		std::string debugName_;
	};

}
