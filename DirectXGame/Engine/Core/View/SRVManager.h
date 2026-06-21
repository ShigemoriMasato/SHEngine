#pragma once
#include "ViewFunc.h"
#include <cstdint>
#include <vector>
#include <wrl.h>

class SRVManager;

/**
 * @class SRVHandle
 * @brief SRV（Shader GPUBuffer View）ハンドルを管理するクラス
 * 
 * SRVディスクリプタヒープ内の位置を保持し、
 * CPUとGPUディスクリプタハンドルへのアクセスを提供する。
 */
class SRVHandle final {
public:
	/// @brief デフォルトコンストラクタ
	SRVHandle() = default;
	/// @brief デストラクタ
	~SRVHandle();

	/**
	 * @brief ハンドルを更新
	 * @param manager SRVマネージャー
	 * @param operateIndex 操作インデックス（-1の場合は自動割り当て）
	 */
	void UpdateHandle(SRVManager* manager, int operateIndex = -1);

	/// @brief CPUディスクリプタハンドルを取得
	/// @return D3D12_CPU_DESCRIPTOR_HANDLE
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPU() const { return CPU; }
	/// @brief GPUディスクリプタハンドルを取得
	/// @return D3D12_GPU_DESCRIPTOR_HANDLE
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPU() const { return GPU; }
	/// @brief オフセットを取得
	/// @return オフセット値
	int GetHandle() const { return offset_; }

private:

	/// @brief CPUディスクリプタハンドル
	D3D12_CPU_DESCRIPTOR_HANDLE CPU;
	/// @brief GPUディスクリプタハンドル
	D3D12_GPU_DESCRIPTOR_HANDLE GPU;
	/// @brief ディスクリプタヒープ内のオフセット
	int offset_;

	/// @brief 管理するSRVマネージャー
	SRVManager* manager_;

	/// @brief ハンドルが更新済みかどうか
	bool isUpdated_ = false;
};

/**
 * @class SRVManager
 * @brief SRVディスクリプタヒープを管理するクラス
 * 
 * SRV用のディスクリプタヒープを作成・管理し、
 * 空きスロットの割り当てや解放を行う。
 */
class SRVManager {
public:

	/**
	 * @brief コンストラクタ
	 * @param device DirectX12デバイス
	 * @param size ディスクリプタのサイズ
	 * @param num 最大ディスクリプタ数
	 */
	SRVManager(ID3D12Device* device, uint32_t size, int num);
	/// @brief デストラクタ
	~SRVManager() = default;

	/// @brief ディスクリプタヒープを取得
	/// @return ID3D12DescriptorHeap*
	ID3D12DescriptorHeap* GetHeap() { return heap_.Get(); }

	/// @brief ヒープの開始GPUアドレスを取得
	/// @return D3D12_GPU_DESCRIPTOR_HANDLE
	D3D12_GPU_DESCRIPTOR_HANDLE GetStartPtr() { return heap_->GetGPUDescriptorHandleForHeapStart(); }

	/**
	 * @brief 次に使用可能なオフセットを取得
	 * @param startIndex 検索開始インデックス
	 * @return 使用可能なオフセット
	 */
	uint32_t GetNextOffset(int startIndex);

private:

	friend class SRVHandle;

	/// @brief SRVディスクリプタヒープ
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> heap_ = nullptr;

	/// @brief 各スロットが使用中かどうかのフラグ
	std::vector<bool> isUsed_;

	/// @brief ディスクリプタのサイズ
	const uint32_t descriptorSizeSRV;
	/// @brief 最大ディスクリプタ数
	const uint32_t maxCount_;

};
