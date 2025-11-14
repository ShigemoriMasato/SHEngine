#pragma once
#include "Shelf/ShaderShelf.h"
#include "Shelf/BlendStateShelf.h"
#include "Shelf/DepthStencilShelf.h"
#include "Shelf/RasterizerShelf.h"
#include "Shelf/RootSignatureShelf.h"
#include "Shelf/InputLayoutShelf.h"


struct PSOConfig {
	std::string vs = "Object3d.VS.hlsl";
	std::string ps = "Object3d.PS.hlsl";
	RootSignatureID rootID = RootSignatureID::Default;
	InputLayoutID inputLayoutID = InputLayoutID::Default;
	BlendStateID blendID = BlendStateID::Normal;
	DepthStencilID depthStencilID = DepthStencilID::Default;
	RasterizerID rasterizerID = RasterizerID::Fill;
	D3D12_PRIMITIVE_TOPOLOGY topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	bool isSwapChain = true;

	bool operator==(const PSOConfig& other) const {
		return vs == other.vs &&
			ps == other.ps &&
			blendID == other.blendID &&
			depthStencilID == other.depthStencilID &&
			rasterizerID == other.rasterizerID &&
			rootID == other.rootID &&
			inputLayoutID == other.inputLayoutID &&
			topology == other.topology &&
			isSwapChain == other.isSwapChain;
	}

	bool operator!=(const PSOConfig& other) const {
		return !(*this == other);
	}
};

namespace std {
	template<>
	struct hash<PSOConfig> {
		size_t operator()(const PSOConfig& cfg) const {
			size_t h = 0;
			hash_combine(h, hash<string>()(cfg.vs));
			hash_combine(h, hash<string>()(cfg.ps));
			hash_combine(h, hash<int>()(static_cast<int>(cfg.blendID)));
			hash_combine(h, hash<int>()(static_cast<int>(cfg.depthStencilID)));
			hash_combine(h, hash<int>()(static_cast<int>(cfg.rasterizerID)));
			hash_combine(h, hash<int>()(static_cast<int>(cfg.rootID)));
			hash_combine(h, hash<int>()(static_cast<int>(cfg.inputLayoutID)));
			hash_combine(h, hash<int>()(static_cast<int>(cfg.topology)));
			hash_combine(h, hash<bool>()(cfg.isSwapChain));
			return h;
		}

	private:
		// シンプルな hash_combine 実装（Boost風）
		static void hash_combine(size_t& seed, size_t value) {
			seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		}
	};
}
