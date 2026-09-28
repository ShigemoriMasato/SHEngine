#pragma once
#include "ViewFunc.h"
#include <cstdint>
#include <vector>
#include <wrl.h>

class DSVManager;

/**
 * @class DSVHandle
 * @brief DSV(深度ステンシルビュー)のハンドルクラス
 * 
 * DSVヒープ内の特定の位置を指すハンドル。
 * CPUとGPUの両方のディスクリプタハンドルを保持し、
 * デストラクタで自動的にリソースを解放する。
 */
class DSVHandle final {
public:
	DSVHandle() = default;
	~DSVHandle();

	/**
	 * @brief ハンドルの更新
	 * 
	 * DSVManagerから新しいオフセットを取得し、CPU/GPUハンドルを設定する。
	 * 
	 * @param manager DSVManagerのポインタ
	 * @param operateIndex 開始インデックス(デフォルト0)
	 */
	void UpdateHandle(DSVManager* manager, int operateIndex = 0);

	/**
	 * @brief CPUディスクリプタハンドルの取得
	 * @return CPUディスクリプタハンドル
	 */
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPU() const { return CPU; }

	/**
	 * @brief GPUディスクリプタハンドルの取得
	 * @return GPUディスクリプタハンドル
	 */
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPU() const { return GPU; }

	/**
	 * @brief オフセットの取得
	 * @return ヒープ内のオフセット
	 */
	int GetHandle() const { return offset_; }

private:

	/// @brief CPUディスクリプタハンドル
	D3D12_CPU_DESCRIPTOR_HANDLE CPU = { 0 };
	/// @brief GPUディスクリプタハンドル
	D3D12_GPU_DESCRIPTOR_HANDLE GPU = { 0 };
	/// @brief ヒープ内のオフセット
	int offset_{};

	/// @brief 関連付けられたDSVManager
	DSVManager* manager_{};

	/// @brief ハンドルが更新済みかどうか
	bool isUpdated_ = false;
};

/**
 * @class DSVManager
 * @brief DSV(深度ステンシルビュー)ヒープを管理するクラス
 * 
 * DSV用のディスクリプタヒープを作成し、個々のハンドルの割り当てを管理する。
 * ヒープ内の使用状況を追跡し、空きスロットを効率的に割り当てる。
 */
class DSVManager {
public:

	/**
	 * @brief DSVManagerのコンストラクタ
	 * 
	 * DSVヒープを作成し、管理用のデータ構造を初期化する。
	 * 
	 * @param device DirectX12デバイス
	 * @param size ディスクリプタのサイズ
	 * @param num ヒープ内のディスクリプタ数
	 */
	DSVManager(ID3D12Device* device, uint32_t size, int num);
	~DSVManager() = default;

	/**
	 * @brief ヒープの取得
	 * @return DSVディスクリプタヒープ
	 */
	ID3D12DescriptorHeap* GetHeap() { return heap_.Get(); }

	/**
	 * @brief ヒープの開始GPUポインタの取得
	 * @return ヒープの開始GPUディスクリプタハンドル
	 */
	D3D12_GPU_DESCRIPTOR_HANDLE GetStartPtr() { return heap_->GetGPUDescriptorHandleForHeapStart(); }

	/**
	 * @brief 次の空きオフセットの取得
	 * 
	 * 指定されたインデックス以降で最初の未使用スロットを見つける。
	 * 
	 * @param startIndex 検索開始インデックス
	 * @return 空きスロットのオフセット
	 */
	uint32_t GetNextOffset(int startIndex);

private:

	friend class DSVHandle;

	/// @brief DSVディスクリプタヒープ
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> heap_ = nullptr;

	/// @brief 各スロットの使用状況
	std::vector<bool> isUsed_;

	/// @brief DSVディスクリプタのサイズ
	const uint32_t descriptorSizeDSV = 0;
	/// @brief ヒープ内の最大ディスクリプタ数
	const uint32_t maxCount_;

};


