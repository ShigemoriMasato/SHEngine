#pragma once
#include <string>
#include <d3d12.h>
#include <Core/View/SRVManager.h>
#include <utility>
#include <Utility/Vector.h>
#include <DirectXTex/DirectXTex.h>


namespace SHEngine {

	enum class Format : uint32_t {
		R8_UNORM,
		R8G8B8A8_UNORM,
		R32G32B32A32_UINT,
		R32_UINT,

	};

	class TextureData {
	public:

		enum class Type {
			Normal,
			DDS,
		};

		TextureData() = default;
		~TextureData() = default;

		//明示的な解放
		void Release();

		int GetHandle() const { return srvHandle_.GetHandle(); }
		D3D12_GPU_DESCRIPTOR_HANDLE GetSRVHandle() const { return srvHandle_.GetGPU(); }
		D3D12_GPU_DESCRIPTOR_HANDLE GetUAVHandle() const { return uavHandle_.GetGPU(); }
		ID3D12Resource* GetResource() const { return textureResource_.Get(); }
		std::pair<uint32_t, uint32_t> GetSize() const { return { width_, height_ }; }
		Vector4 GetClearColor() const { return clearColor_; }
		bool IsUnordered() const { return unordered_; }
		DXGI_FORMAT GetFormat() const { return format_; }

	private:

		friend class TextureManager;
		//TextureDataの読み込み
		DirectX::ScratchImage Create(std::string filePath, ID3D12Device* device, SRVManager* srvManager);
		//Window用のテクスチャを作成
		void Create(uint32_t width, uint32_t height, Vector4 clearColor, Format format, bool unordered, ID3D12Device* device, SRVManager* srvManager);
		//SwapChain用のテクスチャを作成
		void Create(ID3D12Resource* resource, ID3D12Device* device, SRVManager* manager, uint32_t clearColor);
		//Depth用のテクスチャを作成
		void Create(ID3D12Resource* resource, ID3D12Device* device, SRVManager* manager);
		//空っぽのテクスチャを作成
		void Create(uint32_t width, uint32_t height, ID3D12Device* device, SRVManager* srvManager);

	private:

		static inline int debugTextureCount = 0;

		uint32_t width_ = 0;
		uint32_t height_ = 0;

		Microsoft::WRL::ComPtr<ID3D12Resource> textureResource_ = nullptr;

		SRVHandle srvHandle_{};
		SRVHandle uavHandle_{};
		Vector4 clearColor_{};

		Type type_ = Type::Normal;

		TextureManager* textureManager_ = nullptr;

		DXGI_FORMAT format_ = DXGI_FORMAT_R8G8B8A8_UNORM;

		bool unordered_ = false;
	};

}
