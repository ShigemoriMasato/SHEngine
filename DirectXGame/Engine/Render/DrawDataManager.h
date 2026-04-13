#pragma once
#include <Core/DXDevice.h>
#include <Utility/DirectUtilFuncs.h>

namespace SHEngine {

	/**
	 * @struct DrawData
	 * @brief 描画に必要なデータをまとめた構造体
	 */
	struct DrawData {
		/// @brief 頂点バッファビューの配列
		std::vector<D3D12_VERTEX_BUFFER_VIEW> vbv;
		/// @brief インデックスバッファビュー
		D3D12_INDEX_BUFFER_VIEW ibv;
		/// @brief 頂点数
		uint32_t vertexNum;
		/// @brief インデックス数
		uint32_t indexNum;
	};

	/**
	 * @class DrawDataManager
	 * @brief 描画データ（頂点バッファ、インデックスバッファ）を管理するクラス
	 *
	 * 頂点データとインデックスデータをGPUリソースに変換し、
	 * 描画に必要なDrawDataを作成・管理する。
	 */
	class DrawDataManager {
	public:

		void Initialize(DXDevice* device);

		template <typename T>
		void AddVertexBuffer(const std::vector<T>& data);
		void AddIndexBuffer(std::vector<uint32_t> indices);
		int CreateDrawData();

		DrawData GetDrawData(int index) const {
			assert(index >= 0 && index < static_cast<int>(drawDataList_.size()) && "DrawDataManager::GetDrawData: Invalid DrawData index");
			return drawDataList_[index].first;
		}

		//即席
		void CopyBufferData(int drawDataIndex, const void* data, size_t size) {
			assert(drawDataIndex >= 0 && drawDataIndex < static_cast<int>(drawDataList_.size()) && "DrawDataManager::CoppyBufferData: Invalid DrawData index");
			MapDataForBin& mapData = drawDataList_[drawDataIndex].second;
			assert(size <= mapData.size && "DrawDataManager::CopyBufferData: Data size exceeds mapped buffer size");
			std::memcpy(mapData.mapped, data, size);
		}

	private:

		/// @brief DirectX12デバイスへのポインタ
		DXDevice* device_ = nullptr;
		/// @brief ロガー
		Logger logger_ = nullptr;

		/**
		 * @struct MapDataForBin
		 * @brief マップされたバッファ情報
		 */
		struct MapDataForBin {
			/// @brief マップされたメモリへのポインタ
			void* mapped = nullptr;
			/// @brief バッファサイズ
			size_t size = 0;
		};
		/// @brief 頂点バッファビューの一時リスト
		std::vector<D3D12_VERTEX_BUFFER_VIEW> vertexBufferViews_;
		/// @brief インデックスバッファビュー
		D3D12_INDEX_BUFFER_VIEW indexBufferView_;
		/// @brief マップデータ
		MapDataForBin mapData_;

		/// @brief 頂点数
		uint32_t vertexNum_ = 0;
		/// @brief インデックス数
		uint32_t indexNum_ = 0;
		/// @brief DrawDataとMapDataのペアのリスト
		std::vector<std::pair<DrawData, MapDataForBin>> drawDataList_;

		/**
		 * @struct GPUBuffer
		 * @brief リソースラッパー
		 */
		struct Resource {
			/// @brief ID3D12ResourceのComポインタ
			Microsoft::WRL::ComPtr<ID3D12Resource> res;
		};
		/// @brief リソースのリスト
		std::vector<Resource> resources_;

	};

	template<typename T>
	inline void DrawDataManager::AddVertexBuffer(const std::vector<T>& data) {
		Resource res{};
		size_t bufferSize = sizeof(T) * data.size();
		res.res.Attach(Func::CreateBufferResource(device_->GetDevice(), bufferSize));	//256の倍数に揃えたうえでnum倍
		resources_.push_back(res);

		//データ転送
		mapData_.size = bufferSize;

		HRESULT hr = res.res->Map(0, nullptr, &mapData_.mapped);
		assert(SUCCEEDED(hr));
		std::memcpy(mapData_.mapped, data.data(), mapData_.size);
		res.res->Unmap(0, nullptr);

		//VBV作成
		D3D12_VERTEX_BUFFER_VIEW vbv{};
		vbv.BufferLocation = res.res->GetGPUVirtualAddress();
		vbv.SizeInBytes = static_cast<UINT>(bufferSize);
		vbv.StrideInBytes = sizeof(T);

		vertexBufferViews_.push_back(vbv);
		vertexNum_ = static_cast<int>(data.size());

		logger_->debug("Vertex Buffer Added: {} vertices", data.size());
	}

}
