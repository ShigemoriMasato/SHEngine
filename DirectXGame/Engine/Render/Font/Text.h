#pragma once
#include <Assets/Fonts/FontLoader.h>
#include <Render/Renderer.h>

namespace SHEngine {

	class Text {
	public:

		static void SetFontLoader(FontLoader* fontLoader) { fontLoader_ = fontLoader; }

		Text(int maxCharNum = 128) : maxCharNum_(maxCharNum) {};

		void Initialize(const Mesh& plane, const std::string& fontPath, int fontSize = 64, std::string debugName = "");

		void SetText(const std::wstring& text);
		void SetSize(float size);
		void SetTransform(const Transform& transform);
		void SetColor(const Vector4& color);
		void Update(Matrix4x4 vpMat);
		void Draw(DCC* direct);

	private:

		static inline FontLoader* fontLoader_ = nullptr;

		int textureIndex_ = -1;

		std::string fontPath_ = "";
		int fontSize_ = 0;

		std::unique_ptr<SHEngine::BufferContainer> container_ = nullptr;
		std::unique_ptr<SHEngine::Renderer> renderer_ = nullptr;

		SHEngine::GPUBuffer* matrixBuffer_ = nullptr;
		SHEngine::GPUBuffer* charPositionBuffer_ = nullptr;
		SHEngine::GPUBuffer* textureIndexBuffer_ = nullptr;
		SHEngine::GPUBuffer* colorBuffer_ = nullptr;

		std::vector<CharPosition> charPositions_{};

		Matrix4x4 charSizeMat_ = Matrix4x4::Identity();
		Matrix4x4 worldMat_ = Matrix4x4::Identity();

		const int maxCharNum_;

	};

}
