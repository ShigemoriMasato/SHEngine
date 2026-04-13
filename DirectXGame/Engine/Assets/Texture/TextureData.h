#pragma once
#include <string>
#include <d3d12.h>
#include <Core/View/SRVManager.h>
#include <utility>
#include <Utility/Vector.h>
#include <DirectXTex/DirectXTex.h>


namespace SHEngine {


	class TextureData {
	public:

		TextureData() = default;
		~TextureData() = default;

		//明示的な解放
		void Release();

		int GetOffset() const { return srvHandle_.GetOffset(); }
		D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle() const { return srvHandle_.GetGPU(); }
		ID3D12Resource* GetResource() const { return textureResource_.Get(); }
		std::pair<uint32_t, uint32_t> GetSize() const { return { width_, height_ }; }
		Vector4 GetClearColor() const { return clearColor_; }

	private:

		friend class TextureManager;
		Microsoft::WRL::ComPtr<ID3D12Resource> Create(std::string filePath, ID3D12Device* device, SRVManager* srvManager, ID3D12GraphicsCommandList* cmdList);
		//Window用のテクスチャを作成
		void Create(uint32_t width, uint32_t height, Vector4 clearColor, ID3D12Device* device, SRVManager* srvManager);
		//SwapChain用のテクスチャを作成
		void Create(ID3D12Resource* resource, ID3D12Device* device, SRVManager* manager, uint32_t clearColor);
		//Depth用のテクスチャを作成
		void Create(ID3D12Resource* resource, ID3D12Device* device, SRVManager* manager);
		//ビットマップテクスチャを作成
		Microsoft::WRL::ComPtr<ID3D12Resource> Create(uint32_t width, uint32_t height, std::vector<uint32_t> colorMap, ID3D12Device* device, SRVManager* srvManager, ID3D12GraphicsCommandList* cmdList);

	private:

		static inline int debugTextureCount = 0;

		uint32_t width_ = 0;
		uint32_t height_ = 0;

		Microsoft::WRL::ComPtr<ID3D12Resource> textureResource_ = nullptr;

		SRVHandle srvHandle_{};
		Vector4 clearColor_{};

		TextureManager* textureManager_ = nullptr;
	};

}
