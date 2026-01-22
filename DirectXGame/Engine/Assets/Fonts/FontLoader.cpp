#include "FontLoader.h"

#include <algorithm>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <Utility/Vector.h>
#include <Utility/ConvertString.h>

void FontLoader::Initialize(TextureManager* textureManager) {
	logger_ = getLogger("Engine");
	logger_->info("FontLoader Initialize");

	LoadResponseText();

	textureManager_ = textureManager;
	FT_Error err = FT_Init_FreeType(&ft_);
	if (err) {
		logger_->error("Failed to initialize FreeType Library");
		assert(false);
		return;
	}
}

int FontLoader::Load(const std::string& filePath, int fontSize) {
	//stringの整合性チェック
	std::filesystem::path factPath = FilePathChecker(filePath);
	std::string cacheFileName = factPath.filename().string() + "_" + std::to_string(fontSize);

	if (fontIndex_.find(cacheFileName) != fontIndex_.end()) {
		return fontIndex_[cacheFileName].textureIndex;
	}

	logger_->info("FontLoader Load: {}", factPath.string());

	auto values = binaryManager_.Read(cachePath_ + cacheFileName);
	FontData fontBuffer;

	//キャッシュデータを何らかの方法で作成する
	if (values.empty()) {
		//キャッシュが存在しない場合
		//フォントデータを新規作成
		fontBuffer = CreateFontBuffer(factPath.string(), fontSize);
		//キャッシュを作成仕様と思ったが現状げろクソ重くなるのでやりません
	} else {
		//キャッシュが存在する場合
		int index = 0;
		LoadCache(values, fontBuffer);
	}

	int textureID = textureManager_->CreateBitmapTexture(atlas_width_, atlas_height_, fontBuffer.atlasData);
	fontBuffer.textureIndex = textureID;
	fontIndex_[cacheFileName] = fontBuffer;

	return textureID;
}

CharPosition FontLoader::GetCharPosition(const std::string& filePath, wchar_t character, int fontSize) {
	std::filesystem::path factPath(FilePathChecker(filePath));
	std::string cacheFileName = factPath.filename().string() + "_" + std::to_string(fontSize);

	if (fontIndex_.find(cacheFileName) == fontIndex_.end()) {
		logger_->error("FontLoader GetCharPosition Font Not Loaded: {}", factPath.string());
		assert(false);
		return {};
	}

	auto& fontData = fontIndex_[cacheFileName];
	if (fontData.charPositions.find(character) != fontData.charPositions.end()) {
		return fontData.charPositions[character];
	}

	return fontData.charPositions[L'□'];
}

FontLoader::FontData FontLoader::CreateFontBuffer(const std::string& filePath, int fontSize) {
	FontData data{};
	data.atlasData.resize(atlas_width_ * atlas_height_, 0);

	FT_Face face = nullptr;

	FT_Error err = FT_New_Face(ft_, filePath.c_str(), 0, &face);
	assert(!err);
	err = FT_Set_Pixel_Sizes(face, 0, fontSize);
	assert(!err);

	//文字の最大の高さを取得
	int maxTop = 0;
	int maxBottom = 0;

	for (wchar_t c : text_) {
		FT_Load_Char(face, c, FT_LOAD_RENDER);
		auto& g = face->glyph;

		maxTop = std::max(maxTop, g->bitmap_top);
		maxBottom = std::max(maxBottom, (int)g->bitmap.rows - g->bitmap_top);
	}

	int height = maxTop + maxBottom;

	//文字を配置する位置
	int dest_x = 0;
	int dest_y = 0;

	for (const wchar_t& c : text_) {
		//フォントの文字を持ってくる
		FT_Load_Char(face, c, FT_LOAD_RENDER);

		auto& bitmap = face->glyph->bitmap;

		//改行処理
		if (dest_x + (int)bitmap.width >= atlas_width_) {
			dest_x = 0;
			dest_y += height + 2;

			//Atlasの高さを超えたら終了
			if (dest_y + height >= atlas_height_) {
				break;
			}
		}

		for (int y = 0; y < (int)bitmap.rows; ++y) {
			for (int x = 0; x < (int)bitmap.width; ++x) {
				uint8_t alpha = bitmap.buffer[y * bitmap.pitch + x];
				if (alpha == 0) continue;

				//下線を揃えるためにmaxTopを基準にしてyをオフセットさせる
				int offsetY = maxTop - face->glyph->bitmap_top;
				int targetIndex = (dest_y + offsetY + y) * atlas_width_ + (dest_x + x);

				data.atlasData[targetIndex] = 0xffffff00 | alpha;
				data.lastUsedIndex = std::max(data.lastUsedIndex, targetIndex);

			}
		}

		CharPosition& uv = data.charPositions[c];
		int offsetY = maxTop - face->glyph->bitmap_top;

		int glyphTop = dest_y + offsetY;
		int glyphBottom = glyphTop + bitmap.rows;
		int glyphLeft = dest_x;
		int glyphRight = dest_x + bitmap.width;

		uv.uvStart = { float(glyphLeft) / float(atlas_width_), float(glyphTop) / float(atlas_height_) };
		uv.uvEnd = { float(glyphRight) / float(atlas_width_), float(glyphBottom) / float(atlas_height_) };
		uv.advanceX = static_cast<float>(face->glyph->advance.x) / 64.f;
		uv.bearingY = float(face->glyph->bitmap_top);
		uv.bearingX = float(face->glyph->bitmap_left);

		//次の文字の位置を更新
		dest_x += bitmap.width + 2;
	}

	logger_->info("LastUsedIndex: {}", data.lastUsedIndex);
	logger_->info("leftHeight: {}", atlas_height_ - dest_y);

	return data;
}

void FontLoader::LoadResponseText() {
	std::ifstream file(responseTextFile_);

	if (!file.is_open()) {
		logger_->error("FontLoader LoadResponseText Failed: {}", responseTextFile_);
		assert(false);
		return;
	}

	std::string line;
	std::getline(file, line);

	file.close();

	std::wstring wLine = ConvertString(line);
	text_ = std::vector<wchar_t>(wLine.begin(), wLine.end());

	//重複削除
	std::sort(text_.begin(), text_.end());
	text_.erase(std::unique(text_.begin(), text_.end()), text_.end());
}

std::string FontLoader::FilePathChecker(const std::string& filePath) {
	bool notAssets = false;

	std::string basePath = "Assets/";

	if (filePath.length() < basePath.length()) {
		notAssets = true;
	} else {
		for (int i = 0; i < int(basePath.length()); ++i) {
			if (filePath[i] != basePath[i]) {
				notAssets = true;
			}
		}
	}

	if (notAssets) {
		return basePath_ + filePath;
	} else {
		return filePath;
	}

	return {};
}

void FontLoader::CreateCache(const FontData& data, const std::string& cacheFileName) {

	//AtlasData
	binaryManager_.RegisterOutput(static_cast<int>(data.lastUsedIndex));
	for (int i = 0; i < data.lastUsedIndex; ++i) {
		binaryManager_.RegisterOutput(uint8_t(data.atlasData[i]) | 0xff);
	}

	//CharPositions
	binaryManager_.RegisterOutput(static_cast<int>(data.charPositions.size()));
	for (const auto& [key, value] : data.charPositions) {
		binaryManager_.RegisterOutput(static_cast<int>(key));
		binaryManager_.RegisterOutput(value.uvStart);
		binaryManager_.RegisterOutput(value.uvEnd);
	}

	if (std::filesystem::exists(cachePath_) == false) {
		std::filesystem::create_directories(cachePath_);
	}

	binaryManager_.Write("cache/" + cacheFileName);
}

void FontLoader::LoadCache(const std::vector<std::shared_ptr<ValueBase>>& values, FontData& data) {
	if (values.empty()) {
		return;
	}

	int index = 0;
	//AtlasData
	data.lastUsedIndex = BinaryManager::Reverse<int>(values[index++].get());
	data.atlasData.resize(atlas_width_ * atlas_height_, 0);
	for (int i = 0; i < data.lastUsedIndex; ++i) {
		data.atlasData[i] = 0xffffff00 | BinaryManager::Reverse<uint8_t>(values[index++].get());
	}
	//CharPositions
	int charPosSize = BinaryManager::Reverse<int>(values[index++].get());
	for (int i = 0; i < charPosSize; ++i) {
		wchar_t key = static_cast<wchar_t>(BinaryManager::Reverse<int>(values[index++].get()));
		Vector2 uvStart = BinaryManager::Reverse<Vector2>(values[index++].get());
		Vector2 uvEnd = BinaryManager::Reverse<Vector2>(values[index++].get());
		CharPosition charPos;
		charPos.uvStart = uvStart;
		charPos.uvEnd = uvEnd;
		data.charPositions[key] = charPos;
	}
}
