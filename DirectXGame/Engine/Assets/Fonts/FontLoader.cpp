#include "FontLoader.h"

#include <algorithm>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <Utility/Vector.h>
#include <Utility/ConvertString.h>

using namespace SHEngine;

void FontLoader::Initialize(TextureManager* textureManager) {
	logger_ = GetLogger("Engine");
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

	FontData fontBuffer;

	//フォントデータを新規作成
	fontBuffer = CreateFontBuffer(factPath.string(), fontSize);

	// colorMapからScratchImageを作成する
	DirectX::ScratchImage scratchImage;
	HRESULT hr = scratchImage.Initialize2D(DXGI_FORMAT_R8G8B8A8_UNORM, atlas_width_, atlas_height_, 1, 1);
	assert(SUCCEEDED(hr));

	const DirectX::Image* img = scratchImage.GetImage(0, 0, 0);

	//データのコピー
	const uint8_t* srcData = reinterpret_cast<const uint8_t*>(fontBuffer.atlasData.data());
	uint8_t* destData = img->pixels;

	size_t rowBytes = atlas_width_ * sizeof(uint32_t);

	//行ごとにコピー
	for (size_t y = 0; y < atlas_height_; ++y) {
		std::memcpy(destData + y * rowBytes, srcData + y * rowBytes, rowBytes);
	}


	int textureID = textureManager_->CreateBitmapTexture(atlas_width_, atlas_height_, scratchImage);
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
		uv.advanceX = static_cast<float>(face->glyph->advance.x) / float(128);
		uv.bearingY = float(face->glyph->bitmap_top);
		uv.bearingX = float(face->glyph->bitmap_left);
		uv.descender = float(face->glyph->metrics.horiBearingY - face->glyph->metrics.height) / float(128);

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

	text_.push_back(L'衝');
	text_.push_back(L'突');
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
