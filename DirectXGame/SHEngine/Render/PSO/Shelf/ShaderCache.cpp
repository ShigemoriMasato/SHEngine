#include "ShaderCache.h"
#include <Tool/Binary/BinaryManager.h>
#include <Tool/Logger/Logger.h>
#include <algorithm>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;
using namespace SHEngine;

namespace {

	// .binの中身(リフレクションの構造など)を変えたら上げること。古いキャッシュは読まれずに再コンパイルされる
	constexpr uint32_t kCacheVersion = 1;

	// "Compute/Emit.CS.hlsl" → "Compute/Emit.CS" (拡張子.hlslを外す)
	std::string RemoveHlslExtension(const std::string& shaderName) {
		fs::path path(shaderName);
		path.replace_extension();
		return path.generic_string();
	}

	std::string NormalizePath(const std::string& path) {
		return fs::path(path).lexically_normal().generic_string();
	}

	// 更新日時を比較用の数値にする。取得できなければfalse
	bool GetWriteTime(const std::string& path, uint64_t& outTime) {
		std::error_code ec;
		auto time = fs::last_write_time(path, ec);
		if (ec) {
			return false;
		}
		outTime = static_cast<uint64_t>(time.time_since_epoch().count());
		return true;
	}

	// ---- リフレクションの書き込み/読み込み ----

	void WriteBuffers(BinaryManager& bm, const std::vector<ShaderBufferBinding>& buffers) {
		bm.Register(uint32_t(buffers.size()));
		for (const auto& b : buffers) {
			bm.Register(b.name);
			bm.Register(b.registerIndex);
			bm.Register(b.space);
			bm.Register(b.size);
			bm.Register(b.hasCounter);
		}
	}

	bool ReadBuffers(BinaryManager& bm, std::vector<ShaderBufferBinding>& buffers) {
		uint32_t count = 0;
		if (!bm.Reverse(count)) return false;
		buffers.resize(count);
		for (auto& b : buffers) {
			bm.Reverse(b.name);
			bm.Reverse(b.registerIndex);
			bm.Reverse(b.space);
			bm.Reverse(b.size);
			bm.Reverse(b.hasCounter);
		}
		return !bm.IsFailed();
	}

	void WriteReflection(BinaryManager& bm, const ShaderReflectionData& data) {
		for (uint32_t size : data.threadGroupSize) {
			bm.Register(size);
		}

		WriteBuffers(bm, data.cbvs);
		WriteBuffers(bm, data.srvs);
		WriteBuffers(bm, data.uavs);

		bm.Register(uint32_t(data.textures.size()));
		for (const auto& t : data.textures) {
			bm.Register(t.name);
			bm.Register(t.registerIndex);
			bm.Register(t.space);
			bm.Register(t.count);
			bm.Register(uint32_t(t.dimension));
			bm.Register(t.isUAV);
		}

		bm.Register(uint32_t(data.samplers.size()));
		for (const auto& s : data.samplers) {
			bm.Register(s.name);
			bm.Register(s.registerIndex);
			bm.Register(s.space);
			bm.Register(s.count);
		}
	}

	bool ReadReflection(BinaryManager& bm, ShaderType shaderType, ShaderReflectionData& data) {
		data = {};
		data.shaderType = shaderType;

		for (uint32_t& size : data.threadGroupSize) {
			bm.Reverse(size);
		}

		if (!ReadBuffers(bm, data.cbvs)) return false;
		if (!ReadBuffers(bm, data.srvs)) return false;
		if (!ReadBuffers(bm, data.uavs)) return false;

		uint32_t count = 0;
		if (!bm.Reverse(count)) return false;
		data.textures.resize(count);
		for (auto& t : data.textures) {
			uint32_t dimension = 0;
			bm.Reverse(t.name);
			bm.Reverse(t.registerIndex);
			bm.Reverse(t.space);
			bm.Reverse(t.count);
			bm.Reverse(dimension);
			bm.Reverse(t.isUAV);
			t.dimension = D3D_SRV_DIMENSION(dimension);
		}

		if (!bm.Reverse(count)) return false;
		data.samplers.resize(count);
		for (auto& s : data.samplers) {
			bm.Reverse(s.name);
			bm.Reverse(s.registerIndex);
			bm.Reverse(s.space);
			bm.Reverse(s.count);
		}

		return !bm.IsFailed();
	}

}

std::string SHEngine::ShaderCache::GetCacheDirectory() {
	return std::string("Compiled/Shader/") + DXDevice::GetShaderCompileConfigName() + "/";
}

bool SHEngine::ShaderCache::Load(const std::string& shaderName, const std::string& sourcePath, ShaderType shaderType, CachedShader& out) {
	const std::string directory = GetCacheDirectory();
	const std::string baseName = RemoveHlslExtension(shaderName);

	BinaryManager bm;
	bm.SetBasePath(directory);
	if (!bm.Boot(baseName + ".bin")) {
		return false;
	}

	// ---- ヘッダー ----
	uint32_t version = 0;
	uint32_t type = 0;
	uint64_t bytecodeSize = 0;
	bm.Reverse(version);
	bm.Reverse(type);
	bm.Reverse(bytecodeSize);
	if (bm.IsFailed() || version != kCacheVersion || type != uint32_t(shaderType)) {
		return false;
	}

	// ---- 依存ファイル(ソース + #include)が更新されていないか ----
	uint32_t dependencyCount = 0;
	if (!bm.Reverse(dependencyCount)) {
		return false;
	}
	for (uint32_t i = 0; i < dependencyCount; ++i) {
		std::string path;
		uint64_t savedTime = 0;
		bm.Reverse(path);
		bm.Reverse(savedTime);
		if (bm.IsFailed()) {
			return false;
		}
		uint64_t currentTime = 0;
		if (!GetWriteTime(path, currentTime) || currentTime != savedTime) {
			return false;
		}
	}

	// ---- リフレクション ----
	if (!ReadReflection(bm, shaderType, out.reflection)) {
		return false;
	}

	// ---- コンパイル済みシェーダー ----
	const std::string csoPath = directory + baseName + ".cso";
	std::ifstream file(csoPath, std::ios::binary);
	if (!file) {
		return false;
	}
	std::error_code ec;
	const auto fileSize = fs::file_size(csoPath, ec);
	if (ec || fileSize != bytecodeSize || fileSize == 0) {
		return false;
	}
	out.bytecode.resize(static_cast<size_t>(fileSize));
	file.read(reinterpret_cast<char*>(out.bytecode.data()), static_cast<std::streamsize>(fileSize));
	if (!file) {
		out.bytecode.clear();
		return false;
	}

	return true;
}

void SHEngine::ShaderCache::Save(const std::string& shaderName, const std::string& sourcePath, ShaderType shaderType,
	const std::vector<uint8_t>& bytecode, const ShaderReflectionData& reflection, const std::vector<std::string>& includes) {
	static Logger logger = GetLogger("Engine");

	if (bytecode.empty() || !reflection.IsValid()) {
		return;
	}

	const std::string directory = GetCacheDirectory();
	const std::string baseName = RemoveHlslExtension(shaderName);

	// ---- 依存ファイルの一覧(重複を除く) ----
	std::vector<std::string> dependencies;
	dependencies.push_back(NormalizePath(sourcePath));
	for (const auto& include : includes) {
		std::string normalized = NormalizePath(include);
		if (std::find(dependencies.begin(), dependencies.end(), normalized) == dependencies.end()) {
			dependencies.push_back(normalized);
		}
	}

	std::vector<std::pair<std::string, uint64_t>> dependencyTimes;
	for (const auto& path : dependencies) {
		uint64_t time = 0;
		if (!GetWriteTime(path, time)) {
			// 更新日時が取れないファイルがあると有効性を判定できないので保存しない
			logger->warn("ShaderCache: Failed to get write time. Cache is not saved. : {}", path);
			return;
		}
		dependencyTimes.emplace_back(path, time);
	}

	// ---- コンパイル済みシェーダー(.cso) ----
	const fs::path csoPath = directory + baseName + ".cso";
	std::error_code ec;
	fs::create_directories(csoPath.parent_path(), ec);
	std::ofstream file(csoPath, std::ios::binary);
	if (!file) {
		logger->warn("ShaderCache: Failed to open file. : {}", csoPath.generic_string());
		return;
	}
	file.write(reinterpret_cast<const char*>(bytecode.data()), static_cast<std::streamsize>(bytecode.size()));
	file.close();

	// ---- リフレクションと依存ファイル情報(.bin) ----
	BinaryManager bm;
	bm.SetBasePath(directory);
	bm.Register(kCacheVersion);
	bm.Register(uint32_t(shaderType));
	bm.Register(uint64_t(bytecode.size()));
	bm.Register(uint32_t(dependencyTimes.size()));
	for (const auto& [path, time] : dependencyTimes) {
		bm.Register(path);
		bm.Register(time);
	}
	WriteReflection(bm, reflection);

	try {
		bm.Write(baseName + ".bin");
	} catch (const std::exception& e) {
		logger->warn("ShaderCache: Failed to write reflection cache. : {}", e.what());
	}
}
