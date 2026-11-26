#pragma once
#include <string>
#include <d3d12.h>
#include <Core/View/SRVManager.h>
#include <Core/View/RTVManager.h>
#include <Core/View/DSVManager.h>
#include <utility>
#include <Utility/Vector.h>
#include <DirectXTex/DirectXTex.h>
#include <Tool/Logger/Logger.h>
#include <Render/Buffer/GPUBuffer.h>

namespace SHEngine {

	enum class Format : uint32_t {
		R8_UNORM,
		R8G8B8A8_UNORM,
		R32G32B32A32_UINT,
		R32_UINT,

	};

	class TextureManager;

	// @brief テクスチャ1枚分のデータ。GPUBufferの派生なので、そのままRenderer/ComputeObjectのSRV(UAV)スロットにセットできる。
	// @note リソースの状態(バリア)はGPUBuffer側で管理される。TransitionBarrier → Flush で切り替えること。
	class TextureData : public GPUBuffer {
	public:

		enum class Type {
			Normal,
			CubeTexture,
		};

		TextureData() = default;
		~TextureData() override = default;

		//明示的な解放
		void Release();

		// @brief テクスチャにはCopyBufferできない
		void CopyBuffer(const void* data, size_t dataSize, std::source_location location = std::source_location::current()) override;
		bool IsTexture() const override { return true; }

		// @brief RTVを持っているか(RenderTextureか)
		virtual bool HasRTV() const { return false; }
		// @brief DSVを持っているか(DepthTextureか)
		virtual bool HasDSV() const { return false; }

		// @brief SRVのディスクリプタヒープ内の位置。バインドレス(t8~)で参照する際のインデックスとしても使用する。
		int GetHandle() const { return srvHandles_.empty() ? -1 : srvHandles_.front()->GetHandle(); }
		D3D12_GPU_DESCRIPTOR_HANDLE GetSRVHandle() const { return srvHandles_.empty() ? D3D12_GPU_DESCRIPTOR_HANDLE{} : srvHandles_.front()->GetGPU(); }
		D3D12_GPU_DESCRIPTOR_HANDLE GetUAVHandle() const { return uavHandles_.empty() ? D3D12_GPU_DESCRIPTOR_HANDLE{} : uavHandles_.front()->GetGPU(); }
		std::pair<uint32_t, uint32_t> GetSize() const { return { width_, height_ }; }
		Vector4 GetClearColor() const { return clearColor_; }
		bool IsUnordered() const { return unordered_; }
		DXGI_FORMAT GetFormat() const { return format_; }
		Type GetType() const { return type_; }
		// @brief 読み込み元のファイルパス(Assets/以下)。ファイルから読み込んでいない場合は空
		const std::string& GetFilePath() const { return filePath_; }

	protected:

		friend class TextureManager;

		// @brief 作成したリソースをGPUBufferとして登録する
		void SetTextureResource(ID3D12Resource* resource, D3D12_RESOURCE_STATES initialState, bool attach);
		// @brief SRVを作成してGPUBufferに登録する
		void CreateSRV(ID3D12Device* device, SRVManager* srvManager, const D3D12_SHADER_RESOURCE_VIEW_DESC& desc);
		// @brief UAVを作成してGPUBufferに登録する
		void CreateUAV(ID3D12Device* device, SRVManager* srvManager, const D3D12_UNORDERED_ACCESS_VIEW_DESC& desc);
		// @brief デバッグ用の名前をリソースにつける
		void SetDebugName(const std::string& prefix);

		//TextureDataの読み込み
		DirectX::ScratchImage Create(std::string filePath, ID3D12Device* device, SRVManager* srvManager);
		//Window用のテクスチャを作成
		void Create(uint32_t width, uint32_t height, Vector4 clearColor, Format format, bool unordered, ID3D12Device* device, SRVManager* srvManager);
		//SwapChain用のテクスチャを作成
		void Create(ID3D12Resource* resource, ID3D12Device* device, SRVManager* manager, uint32_t clearColor);
		//空っぽのテクスチャを作成
		void Create(uint32_t width, uint32_t height, ID3D12Device* device, SRVManager* srvManager);

		static inline int debugTextureCount = 0;
		static inline Logger logger_ = GetLogger("Engine");

		uint32_t width_ = 0;
		uint32_t height_ = 0;

		Vector4 clearColor_{};

		Type type_ = Type::Normal;

		TextureManager* textureManager_ = nullptr;

		DXGI_FORMAT format_ = DXGI_FORMAT_R8G8B8A8_UNORM;

		bool unordered_ = false;

		std::string filePath_;
	};

	// @brief RTV付きのテクスチャ1枚。OffScreen用・SwapChain用の描画先として使用する。
	class RenderTexture final : public TextureData {
	public:

		bool HasRTV() const override { return true; }

		// @brief RTVのCPUハンドル
		D3D12_CPU_DESCRIPTOR_HANDLE GetRTVHandle() const { return rtvHandle_.GetCPU(); }

	private:

		friend class TextureManager;

		// @brief format_のRTVを作成する。Create後に呼ぶこと
		void CreateRTV(ID3D12Device* device, RTVManager* rtvManager);

		RTVHandle rtvHandle_{};
	};

	// @brief DSV付きの深度テクスチャ1枚(D32_FLOAT)。SRVとしてはR32_FLOATで参照できる。
	class DepthTexture final : public TextureData {
	public:

		bool HasDSV() const override { return true; }

		// @brief DSVのCPUハンドル
		D3D12_CPU_DESCRIPTOR_HANDLE GetDSVHandle() const { return dsvHandle_.GetCPU(); }

	private:

		friend class TextureManager;

		// @brief 深度テクスチャを作成する
		void CreateDepth(uint32_t width, uint32_t height, ID3D12Device* device, SRVManager* srvManager, DSVManager* dsvManager);

		DSVHandle dsvHandle_{};
	};

}
