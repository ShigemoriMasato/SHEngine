#include "CurveAssetIO.h"
#include "../CurveEditorLog.h"
#include <Tool/Binary/BinaryManager.h>
#include <algorithm>
#include <array>
#include <cctype>
#include <filesystem>

namespace fs = std::filesystem;

bool CurveAssetIO::Load(CurveAsset& outAsset, const std::string& categoryName, const std::string& itemName) {
	const std::string path = GetRelativePath(categoryName, itemName);
	BinaryManager bin;
	if (!bin.Boot(path)) {
		CurveEditorLog::Get()->error("Load failed (file not found or BinaryManager version mismatch): {}", path);
		return false;
	}
	if (!Read(bin, outAsset)) {
		CurveEditorLog::Get()->error("Load failed (invalid data): {}", path);
		return false;
	}
	CurveEditorLog::Get()->debug("Loaded: {} (tracks: {})", path, outAsset.GetTracks().size());
	return true;
}

bool CurveAssetIO::Save(const CurveAsset& asset, const std::string& categoryName, const std::string& itemName) {
	const std::string path = GetRelativePath(categoryName, itemName);
	if (!EnsureDirectory(categoryName)) {
		CurveEditorLog::Get()->error("Save failed (cannot create directory): {}", GetDirectory(categoryName));
		return false;
	}
	BinaryManager bin;
	Write(bin, asset);
	try {
		bin.Write(path);
	} catch (const std::exception& e) {
		CurveEditorLog::Get()->error("Save failed: {} ({})", path, e.what());
		return false;
	}
	return true;
}

void CurveAssetIO::Write(BinaryManager& bin, const CurveAsset& asset) {
	const uint32_t version = kFormatVersion;
	const uint32_t trackCount = static_cast<uint32_t>(asset.GetTracks().size());

	bin.Register(kSignature);
	bin.Register(version);
	bin.Register(asset.duration);
	bin.Register(asset.loop);
	bin.Register(trackCount);

	for (const CurveTrack& track : asset.GetTracks()) {
		const uint32_t dimension = static_cast<uint32_t>(track.GetDimension());
		bin.Register(track.GetName());
		bin.Register(dimension);
		for (uint32_t c = 0; c < dimension; ++c) {
			const auto& keys = track.GetChannel(static_cast<int>(c)).GetKeys();
			const uint32_t keyCount = static_cast<uint32_t>(keys.size());
			bin.Register(keyCount);
			for (const CurveKey& key : keys) {
				bin.Register(key.time);
				bin.Register(key.value);
				bin.Register(key.leftHandle);
				bin.Register(key.rightHandle);
			}
		}
	}
}

bool CurveAssetIO::Read(BinaryManager& bin, CurveAsset& outAsset) {
	std::string signature;
	if (!bin.Reverse(signature) || signature != kSignature) {
		CurveEditorLog::Get()->error("Read failed: signature mismatch (\"{}\")", signature);
		bin.Back();
		return false;
	}

	uint32_t version = 0;
	if (!bin.Reverse(version) || version == 0 || version > kFormatVersion) {
		CurveEditorLog::Get()->error("Read failed: unsupported format version {} (supported: 1-{})", version, kFormatVersion);
		return false;
	}

	CurveAsset asset;
	uint32_t trackCount = 0;
	bin.Reverse(asset.duration);
	bin.Reverse(asset.loop);
	bin.Reverse(trackCount);

	for (uint32_t t = 0; t < trackCount && !bin.IsFailed(); ++t) {
		std::string name;
		uint32_t dimension = 0;
		bin.Reverse(name);
		bin.Reverse(dimension);
		if (bin.IsFailed()) {
			CurveEditorLog::Get()->error("Read failed: data is truncated (track {})", t);
			return false;
		}
		if (dimension < 1 || dimension > static_cast<uint32_t>(CurveTrack::kMaxDimension)) {
			CurveEditorLog::Get()->error("Read failed: invalid dimension {} (track \"{}\")", dimension, name);
			return false;
		}

		CurveTrack* track = asset.AddTrack(name, static_cast<int>(dimension));
		if (!track) {
			CurveEditorLog::Get()->error("Read failed: empty or duplicate track name (\"{}\")", name);
			return false;
		}
		for (uint32_t c = 0; c < dimension && !bin.IsFailed(); ++c) {
			CurveChannel& channel = track->GetChannel(static_cast<int>(c));
			uint32_t keyCount = 0;
			bin.Reverse(keyCount);
			for (uint32_t k = 0; k < keyCount && !bin.IsFailed(); ++k) {
				CurveKey key;
				bin.Reverse(key.time);
				bin.Reverse(key.value);
				bin.Reverse(key.leftHandle);
				bin.Reverse(key.rightHandle);
				channel.PushKeyRaw(key);
			}
		}
	}

	// version 2 以降で項目を追加した場合はここで if (version >= 2) { ... } として読む

	if (bin.IsFailed()) {
		CurveEditorLog::Get()->error("Read failed: data is truncated");
		return false;
	}

	asset.Sort();
	outAsset = std::move(asset);
	return true;
}

std::vector<std::string> CurveAssetIO::ListItems(const std::string& categoryName) {
	std::vector<std::string> result;
	std::error_code ec;
	const fs::path dir(GetDirectory(categoryName));
	if (!fs::is_directory(dir, ec)) {
		return result;
	}
	for (const auto& entry : fs::directory_iterator(dir, ec)) {
		if (!entry.is_regular_file(ec)) continue;
		try {
			if (entry.path().extension().string() != kExtension) continue;
			result.push_back(entry.path().stem().string());
		} catch (const std::exception&) {
			// 現在のコードページで表せないファイル名は無視する
			CurveEditorLog::Get()->warn("Skipped a file whose name cannot be converted: {}", GetDirectory(categoryName));
		}
	}
	std::sort(result.begin(), result.end());
	return result;
}

std::string CurveAssetIO::SanitizeName(const std::string& name) {
	std::string result;
	result.reserve(name.size());
	for (char ch : name) {
		const unsigned char uc = static_cast<unsigned char>(ch);
		const bool invalid = uc < 0x20 || ch == '\\' || ch == '/' || ch == ':' || ch == '*' ||
			ch == '?' || ch == '"' || ch == '<' || ch == '>' || ch == '|';
		result.push_back(invalid ? '_' : ch);
	}

	// 前後の空白と末尾のピリオドは Windows で問題になるので削る
	while (!result.empty() && (result.back() == ' ' || result.back() == '.')) result.pop_back();
	while (!result.empty() && result.front() == ' ') result.erase(result.begin());

	if (result.empty()) {
		return "NewCurve";
	}

	// 予約デバイス名
	std::string upper = result;
	std::transform(upper.begin(), upper.end(), upper.begin(),
		[](unsigned char c) { return static_cast<char>(std::toupper(c)); });
	static const std::array<const char*, 22> kReserved = {
		"CON", "PRN", "AUX", "NUL",
		"COM1", "COM2", "COM3", "COM4", "COM5", "COM6", "COM7", "COM8", "COM9",
		"LPT1", "LPT2", "LPT3", "LPT4", "LPT5", "LPT6", "LPT7", "LPT8", "LPT9",
	};
	for (const char* reserved : kReserved) {
		if (upper == reserved) {
			return "_" + result;
		}
	}
	return result;
}

std::string CurveAssetIO::GetRelativePath(const std::string& categoryName, const std::string& itemName,
	const std::string& extension) {
	return kCurveRoot + categoryName + "/" + itemName + extension;
}

std::string CurveAssetIO::GetDirectory(const std::string& categoryName) {
	return kBinaryRoot + kCurveRoot + categoryName + "/";
}

bool CurveAssetIO::EnsureDirectory(const std::string& categoryName) {
	std::error_code ec;
	fs::create_directories(GetDirectory(categoryName), ec);
	if (ec) {
		CurveEditorLog::Get()->error("Failed to create directory: {} ({})", GetDirectory(categoryName), ec.message());
	}
	return !ec;
}
