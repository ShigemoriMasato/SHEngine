#include "Text.h"
#include <Utility/ConvertString.h>

using namespace Matrix;

void SHEngine::Text::Initialize(const Mesh& planeMesh, const std::string& fontPath, int fontSize, std::string debugName) {
	textureIndex_ = fontLoader_->Load(fontPath, fontSize);
	fontPath_ = fontPath;
	fontSize_ = fontSize;

	container_ = std::make_unique<BufferContainer>(4);

	matrixBuffer_ = container_->Create(BufferType::CBV, sizeof(Matrix4x4) * 2);
	charPositionBuffer_ = container_->Create(BufferType::SRV, sizeof(CharPosition), maxCharNum_);
	textureIndexBuffer_ = container_->Create(BufferType::CBV, sizeof(int), 1, BufferNum::Single);
	colorBuffer_ = container_->Create(BufferType::CBV, sizeof(Vector4));
	textureIndexBuffer_->CopyBuffer(&textureIndex_, sizeof(int));

	renderer_ = std::make_unique<Renderer>(SHEngine::VertexType::Default, planeMesh);
	renderer_->SetVS("Engine/FontBasic.VS.hlsl");
	renderer_->SetPS("Engine/FontBasic.PS.hlsl");
	renderer_->SetGPUBuffer(matrixBuffer_, ShaderType::VERTEX_SHADER, BufferType::CBV);
	renderer_->SetGPUBuffer(charPositionBuffer_, ShaderType::VERTEX_SHADER, BufferType::SRV);
	renderer_->SetGPUBuffers({ textureIndexBuffer_, colorBuffer_ }, ShaderType::PIXEL_SHADER, BufferType::CBV);
	renderer_->SetUseTexture(true);
}

void SHEngine::Text::SetText(const std::wstring& text) {
	if (text.size() > maxCharNum_) {
		return;
	}

	charPositions_.reserve(text.size());
	for (wchar_t character : text) {
		charPositions_.push_back(fontLoader_->GetCharPosition(fontPath_, character, fontSize_));
	}
	charPositionBuffer_->CopyBuffer(charPositions_.data(), sizeof(CharPosition) * charPositions_.size());
	renderer_->instanceNum_ = static_cast<uint32_t>(charPositions_.size());

	charPositions_.clear();
}

void SHEngine::Text::SetText(const std::string& text) {
	SetText(ConvertString(text));
}

void SHEngine::Text::SetSize(float size) {
	charSizeMat_ = MakeScaleMatrix({ size, size, 1.0f });
}

void SHEngine::Text::SetTransform(const Transform& transform) {
	worldMat_ = charSizeMat_ * MakeScaleMatrix(transform.scale) * MakeRotationMatrix(transform.rotate) * MakeTranslationMatrix(transform.position);
}

void SHEngine::Text::SetColor(const Vector4& color) {
	colorBuffer_->CopyBuffer(&color, sizeof(Vector4));
}

void SHEngine::Text::SetIsUI(bool isUI) {
	renderer_->SetDepthStencil(isUI ? PSO::DepthStencilID::UI : PSO::DepthStencilID::Default);
	renderer_->SetBlendState(isUI ? PSO::BlendStateID::Force : PSO::BlendStateID::Normal);
}

void SHEngine::Text::Update(Matrix4x4 vpMat) {
	Matrix4x4 mat[2] = { worldMat_, vpMat };
	matrixBuffer_->CopyBuffer(mat, sizeof(Matrix4x4) * 2);
}

void SHEngine::Text::Draw(DCC* direct) {
	renderer_->Draw(direct);
}

float SHEngine::Text::GetLength() const {
	float length = 0.0f;
	for (const auto& charPos : charPositions_) {
		length += charPos.advanceX;
	}
	return length;
}
