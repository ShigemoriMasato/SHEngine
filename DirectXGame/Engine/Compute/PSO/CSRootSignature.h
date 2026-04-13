#pragma once
#include <Core/DXDevice.h>

namespace SHEngine::PSO {

	class CSRootSignature {
	public:

		void Initialize(DXDevice* device);

		ID3D12RootSignature* GetRootSignature(int cbv, int srv, int uav);

	private:

		struct RS {
			Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;
		};
		struct RSConfig {
			int cbvNum = 0;
			int srvNum = 0;
			int uavNum = 0;
			bool operator==(const RSConfig& other) const {
				return cbvNum == other.cbvNum && srvNum == other.srvNum && uavNum == other.uavNum;
			}
			bool operator<(const RSConfig& other) const {
				if (cbvNum != other.cbvNum) return cbvNum < other.cbvNum;
				if (srvNum != other.srvNum) return srvNum < other.srvNum;
				return uavNum < other.uavNum;
			}
		};

		DXDevice* device_ = nullptr;

		std::map<RSConfig, RS> rootSignatures_;

	};

}
