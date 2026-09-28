#pragma once
#include <cstdint>
#include <d3d12.h>

/**
 * @brief ディスクリプタヒープの作成
 * 
 * 指定されたタイプと数のディスクリプタヒープを作成する。
 * 
 * @param device DirectX12デバイス
 * @param type ディスクリプタヒープのタイプ(RTV, DSV, CBV_SRV_UAV等)
 * @param numDescriptors ヒープ内のディスクリプタ数
 * @param shaderVisible シェーダーから可視かどうか
 * @return 作成されたディスクリプタヒープ
 */
ID3D12DescriptorHeap* CreateDescriptorHeap(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type, UINT numDescriptors, bool shaderVisible);

/**
 * @brief CPUディスクリプタハンドルの取得
 * 
 * ディスクリプタヒープ内の指定インデックスのCPUハンドルを取得する。
 * 
 * @param heap ディスクリプタヒープ
 * @param descriptorSize ディスクリプタのサイズ
 * @param index ディスクリプタのインデックス
 * @return CPUディスクリプタハンドル
 */
D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(ID3D12DescriptorHeap* heap, uint32_t descriptorSize, UINT index);

/**
 * @brief GPUディスクリプタハンドルの取得
 * 
 * ディスクリプタヒープ内の指定インデックスのGPUハンドルを取得する。
 * 
 * @param heap ディスクリプタヒープ
 * @param descriptorSize ディスクリプタのサイズ
 * @param index ディスクリプタのインデックス
 * @return GPUディスクリプタハンドル
 */
D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(ID3D12DescriptorHeap* heap, uint32_t descriptorSize, UINT index);
