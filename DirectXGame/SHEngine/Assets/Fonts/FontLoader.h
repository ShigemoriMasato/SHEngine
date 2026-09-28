#pragma once
#include <ft2build.h>
#include FT_FREETYPE_H
#include <unordered_map>
#include <string>

#include <Tool/Binary/BinaryManager.h>
#include <Tool/Logger/Logger.h>
#include <Utility/Vector.h>
#include <Assets/Texture/TextureManager.h>

struct CharPosition {
	Vector2 uvStart;
	Vector2 uvEnd;
	float advanceX;
	float bearingX;
	float bearingY;
	float descender;
};

namespace SHEngine {

	class FontLoader {
	public:

		void Initialize(TextureManager* textureManager);

		int Load(const std::string& filePath, int fontSize = 64);

		CharPosition GetCharPosition(const std::string& filePath, wchar_t character, int fontSize);

	private:

		struct FontData {
			std::vector<uint32_t> atlasData;
			std::unordered_map<wchar_t, CharPosition> charPositions;
			int textureIndex = -1;
			int lastUsedIndex = -1;
		};

	private:

		void LoadResponseText();

		std::string FilePathChecker(const std::string& filePath);

		FontData CreateFontBuffer(const std::string& filePath, int fontSize);

	private://フォント関係
		std::vector<wchar_t> text_;
		FT_Library ft_ = nullptr;
		std::unordered_map<std::string, FT_Face> fonts_;

	private://入出力関係

		std::string basePath_ = "Assets/Fonts/";
		std::string responseTextFile_ = "Assets/.EngineResource/Fonts/response.txt";

	private://アトラス関係

		const int atlas_width_ = 4096;
		const int atlas_height_ = 4096;

	private://フォントデータ
		std::unordered_map<std::string, FontData> fontIndex_;

	private://テクスチャマネージャー
		TextureManager* textureManager_ = nullptr;

	private://Debug
		Logger logger_;
	};

}
