#pragma once
#include <Core/DXDevice.h>
#include <Render/PSO/Shelf/RootSignatureShelf.h>

namespace SHEngine::PSO {

	class CSRootSignature {
	public:

		void Initialize(DXDevice* device, std::map<SamplerID, D3D12_STATIC_SAMPLER_DESC> samplers);

		ID3D12RootSignature* GetRootSignature(int cbv, int srv, int uav, bool useTexture, uint32_t samplerID);

	private:

		struct RS {
			Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;
		};
		struct RSConfig {
			int cbvNum = 0;
			int srvNum = 0;
			int uavNum = 0;
			bool useTexture;
			uint32_t samplerID;
			bool operator==(const RSConfig& other) const {
				return cbvNum == other.cbvNum && srvNum == other.srvNum && uavNum == other.uavNum && useTexture == other.useTexture && samplerID == other.samplerID;
			}
			bool operator<(const RSConfig& other) const {
				if (cbvNum != other.cbvNum) return cbvNum < other.cbvNum;
				if (srvNum != other.srvNum) return srvNum < other.srvNum;
				if(uavNum != other.uavNum) return uavNum < other.uavNum;
				if (useTexture != other.useTexture) return useTexture < other.useTexture;
				return samplerID < other.samplerID;
			}
		};

		DXDevice* device_ = nullptr;

		std::map<RSConfig, RS> rootSignatures_;
		std::map<SamplerID, D3D12_STATIC_SAMPLER_DESC> samplers_;
	};

}
