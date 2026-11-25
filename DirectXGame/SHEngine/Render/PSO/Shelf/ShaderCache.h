#pragma once
#include "ShaderReflection.h"
#include <cstdint>
#include <string>
#include <vector>

namespace SHEngine::ShaderCache {

	// @brief キャッシュから読み込んだシェーダー
	struct CachedShader {
		std::vector<uint8_t> bytecode;
		ShaderReflectionData reflection;
	};

	// @brief キャッシュの保存先のルート ("Compiled/Shader/<コンパイル設定>/")
	std::string GetCacheDirectory();

	// @brief コンパイル済みシェーダーとリフレクションをキャッシュから読み込む
	// @param shaderName Assets/Shader/からの相対パス (例: "Compute/Particle/Emit.CS.hlsl")
	// @param sourcePath コンパイル時に渡すシェーダーファイルのパス
	// @param shaderType シェーダーの種類
	// @param out 読み込んだ結果
	// @return キャッシュが存在し、ソースと#includeしたファイルが保存時から更新されていなければtrue
	bool Load(const std::string& shaderName, const std::string& sourcePath, ShaderType shaderType, CachedShader& out);

	// @brief コンパイル済みシェーダーを "<shaderName>.cso"、リフレクションと依存ファイル情報を "<shaderName>.bin" (BinaryManager) に保存する
	// @param includes #includeで読み込んだファイルのパス
	void Save(const std::string& shaderName, const std::string& sourcePath, ShaderType shaderType,
		const std::vector<uint8_t>& bytecode, const ShaderReflectionData& reflection, const std::vector<std::string>& includes);

}
