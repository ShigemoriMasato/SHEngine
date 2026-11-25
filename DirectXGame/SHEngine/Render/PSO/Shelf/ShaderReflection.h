#pragma once
#include <Core/DXDevice.h>
#include <d3d12shader.h>
#include <cstdint>
#include <string>
#include <vector>

namespace SHEngine {

	// @brief CBV/SRV/UAVのバッファ1つ分のバインド情報
	struct ShaderBufferBinding {
		std::string name;            // HLSL上の変数名
		uint32_t registerIndex = 0;  // レジスタ番号 (b0 → 0)
		uint32_t space = 0;          // レジスタスペース
		// CBV: cbufferのサイズ(byte、16の倍数)
		// SRV/UAV: StructuredBufferの1要素のサイズ(byte)。ByteAddressBuffer/Buffer<T>は0
		uint32_t size = 0;
		bool hasCounter = false;     // Append/Consume/カウンター付きのUAVか
	};

	// @brief テクスチャのバインド情報 (Texture2D / RWTexture2D など。Buffer<T>はSRV/UAV側に入る)
	struct ShaderTextureBinding {
		std::string name;
		uint32_t registerIndex = 0;
		uint32_t space = 0;
		uint32_t count = 1;          // 必要な個数(配列数)。上限なしの配列 (Texture2D tex[]) は0
		D3D_SRV_DIMENSION dimension = D3D_SRV_DIMENSION_UNKNOWN;
		bool isUAV = false;          // RWTextureならtrue
	};

	// @brief サンプラーのバインド情報
	struct ShaderSamplerBinding {
		std::string name;
		uint32_t registerIndex = 0;
		uint32_t space = 0;
		uint32_t count = 1;          // 必要な個数(配列数)。上限なしの配列は0
	};

	// @brief シェーダー1本分のリフレクション結果
	// @note シェーダー内で一度も参照されないリソースはコンパイラに消されるため、ここには出てこない
	struct ShaderReflectionData {
		ShaderType shaderType = ShaderType::Count;

		// 各配列はregisterIndex(→space)の昇順
		std::vector<ShaderBufferBinding> cbvs;
		std::vector<ShaderBufferBinding> srvs;
		std::vector<ShaderBufferBinding> uavs;
		std::vector<ShaderTextureBinding> textures;
		std::vector<ShaderSamplerBinding> samplers;

		// numthreads (Compute / Mesh / Amplification のみ。それ以外は 0,0,0)
		uint32_t threadGroupSize[3] = { 0, 0, 0 };

		// @brief リフレクションが取得できているか
		bool IsValid() const { return shaderType != ShaderType::Count; }

		// @brief ログ出力用の文字列
		std::string ToString() const;
	};

	namespace ShaderReflector {
		// @brief ID3D12ShaderReflectionからShaderReflectionDataを作成する
		// @return 成功したらtrue
		bool Build(ID3D12ShaderReflection* reflection, ShaderType shaderType, ShaderReflectionData& out);
	}

}
