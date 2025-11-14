#include "InsertBarrier.h"

void InsertBarrier(ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_STATES stateAfter, D3D12_RESOURCE_STATES& stateBefore, ID3D12Resource* pResource,
    D3D12_RESOURCE_BARRIER_TYPE type, D3D12_RESOURCE_BARRIER_FLAGS flags) {

    if (stateAfter == stateBefore) {
        // 既に同じ状態ならバリアは不要
        return;
    }

    D3D12_RESOURCE_BARRIER barrier{};
    barrier.Type = type;
    barrier.Flags = flags;
    barrier.Transition.pResource = pResource;
    barrier.Transition.StateBefore = stateBefore;
    barrier.Transition.StateAfter = stateAfter;

    commandList->ResourceBarrier(1, &barrier);

    // 状態を更新
    stateBefore = stateAfter;
}
