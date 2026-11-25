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
		// @brief スレッドグループのサイズを登録(65535を超えた場合、自動的にy->zの順にシフトされる)。自由度を求めるならSetThreadGroupSizeを使用すること
		// @param executeNum 実行するスレッドの総数
		// @param threadSize スレッドグループのXサイズ。Y,Zは1であるものとする。
		void SetExecuteNum(int executeNum, int threadSize = 1);
		// @brief スレッドグループのサイズを登録
		void SetThreadGroupSize(int x, int y = 0, int z = 0) { threadGroupSize_ = { x, y, z }; }
		// @brief CBV/SRV/UAVを登録する。登録した順番がRootParameterIndexになる。
		// @param bufferType 今回使用するBufferの種類(CBV/SRV/UAVのどれか一つ)
		// @param registerNumber シェーダー側のレジスタ番号(b0ならCBVの0)
		// @param buffer 使用するGPUBuffer
		// @note 同じbufferType・registerNumberが既に登録されている場合はBufferを差し替える。
		void SetGPUBuffer(BufferType bufferType, int registerNumber, GPUBuffer* buffer);
		// @brief 複数のCBV/SRV/UAVを、startRegisterから連番で登録する。
		// @param bufferType 今回使用するBufferの種類(CBV/SRV/UAVのどれか一つ)
		// @param startRegister 先頭のBufferのレジスタ番号
		// @param buffers 使用するGPUBufferの配列
		void SetGPUBuffers(BufferType bufferType, int startRegister, const std::vector<GPUBuffer*>& buffers);
		// @brief 登録したCBV/SRV/UAVをすべて解除する。Buffer自体は解放されない。
		void ResetGPUBuffers();
		// @brief テクスチャを使用するかどうかを登録。ComputeShaderでテクスチャを使用する場合はこれをtrueにする必要がある。使用するレジスタはt8~t1031(最大1024個)。
		void SetUseTexture(bool useTexture);
		// @brief SamplerIDをOR演算子で登録。
		void SetSamplerID(uint32_t samplerID) { rootConfig_.samplers = samplerID; }
		void SetSamplerID(SHEngine::PSO::SamplerID samplerID) { rootConfig_.samplers = uint32_t(samplerID); }

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

		// rootConfig_.rootParamsと同じ順番で対応するBufferを保持する
		std::vector<GPUBuffer*> gpuBuffers_;
		PSO::RootSignatureConfig rootConfig_;
		std::string computeShaderName_;
		struct ThreadGroupSize {
			int x;
			int y;
			int z;
		} threadGroupSize_;

		Logger logger_ = GetLogger("Engine");
		std::string debugName_;
	};

}
