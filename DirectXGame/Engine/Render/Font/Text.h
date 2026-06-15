#pragma once
#include <Assets/Fonts/FontLoader.h>
#include "../RenderObject.h"

namespace SHEngine {

	class Text {
	public:

		static void SetFontLoader(FontLoader* fontLoader) { fontLoader_ = fontLoader; }

		Text(int maxCharNum = 128) : maxCharNum_(maxCharNum) {};

		void Initialize(DrawData& planeDrawData, const std::string& fontPath, int fontSize = 64, std::string debugName = "");

		void SetText(const std::wstring& text);
		void SetSize(float size);
		void SetTransform(const Transform& transform);
		void SetColor(const Vector4& color) { color_ = color; }
		void Update(Matrix4x4 vpMat);
		void Draw(Command::Object* cmdObj);

	private:

		static inline FontLoader* fontLoader_ = nullptr;

		int textureIndex_ = -1;

		std::string fontPath_ = "";
		int fontSize_ = 0;

		std::unique_ptr<RenderObject> render_ = nullptr;
		std::vector<CharPosition> charPositions_{};
		Vector4 color_ = { 1.0f, 1.0f, 1.0f, 1.0f };

		Matrix4x4 charSizeMat_ = Matrix4x4::Identity();
		Matrix4x4 worldMat_ = Matrix4x4::Identity();

		const int maxCharNum_;

	};

}
