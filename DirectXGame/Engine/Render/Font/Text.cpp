#include "Text.h"

using namespace Matrix;

void SHEngine::Text::Initialize(DrawData& planeDrawData, const std::string& fontPath, int fontSize, std::string debugName) {
	textureIndex_ = fontLoader_->Load(fontPath, fontSize);
	fontPath_ = fontPath;
	fontSize_ = fontSize;

	render_ = std::make_unique<RenderObject>(debugName);
	render_->Initialize();
	render_->SetDrawData(planeDrawData);
	render_->psoConfig_.vs = "Engine/FontBasic.VS.hlsl";
	render_->psoConfig_.ps = "Engine/FontBasic.PS.hlsl";
	render_->CreateCBV(sizeof(Matrix4x4) * 2, ShaderType::VERTEX_SHADER, "Matrix");
	render_->CreateSRV(sizeof(CharPosition), maxCharNum_, ShaderType::VERTEX_SHADER, "CharConfig");
	render_->CreateCBV(sizeof(int), ShaderType::PIXEL_SHADER, "TextureIndex");
	render_->CreateCBV(sizeof(Vector4), ShaderType::PIXEL_SHADER, "Color");
	render_->SetUseTexture(true);
}

void SHEngine::Text::SetText(const std::wstring& text) {
	if(text.size() > maxCharNum_) {
		return;
	}

	charPositions_.clear();
	for (wchar_t character : text) {
		charPositions_.push_back(fontLoader_->GetCharPosition(fontPath_, character, fontSize_));
	}
	render_->instanceNum_ = static_cast<uint32_t>(charPositions_.size());
}

void SHEngine::Text::SetSize(float size) {
	charSizeMat_ = MakeScaleMatrix({ size, size, 1.0f });
}

void SHEngine::Text::SetTransform(const Transform& transform) {
	worldMat_ = charSizeMat_ * MakeScaleMatrix(transform.scale)* MakeRotationMatrix(transform.rotate)* MakeTranslationMatrix(transform.position);
}

void SHEngine::Text::Update(Matrix4x4 vpMat) {
	Matrix4x4 mat[2] = { worldMat_, vpMat };
	render_->CopyBufferData(0, mat, sizeof(mat));
	render_->CopyBufferData(1, charPositions_.data(), sizeof(CharPosition) * charPositions_.size());
	render_->CopyBufferData(2, &textureIndex_, sizeof(textureIndex_));
	render_->CopyBufferData(3, &color_, sizeof(Vector4));
}

void SHEngine::Text::Draw(Command::Object* cmdObj) {
	render_->Draw(cmdObj);
}
