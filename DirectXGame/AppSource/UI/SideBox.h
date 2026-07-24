#pragma once
#include <Render/Renderer.h>
#include <Render/Font/Text.h>
#include <SHEngine.h>

class SideBox {
public:

	struct Config {
		Vector2 scale = { 1.0f, 1.0f };
		Vector2 position = { 0.0f, 0.0f };
		Vector4 color = { 1.0f, 1.0f, 1.0f };
		Vector2 putDirection = { -1.0f, 0.0f };

		void DrawImGui();
		
		void Save(BinaryManager& bin) const;
		void Load(BinaryManager& bin);
	};

	static void StaticInitialize(SHEngine::Engine* engine);
	static void DrawImGui();

	void Initialize(std::string sentence);
	void IsSelected(bool isSelected) { isSelected_ = isSelected; }
	bool Update(float deltaTime, Camera* orthoCamera, Vector2 mousePos);
	bool IsSelected() const { return isUpdateSelected_; }
	void Draw(DCC* dcc);

	void SetConfig(const Config& config);

private:

	static void SaveCommon();
	static void LoadCommon();

private:

	bool IsMouseInBox(Vector2 mousePos) const;

private:

	static inline const std::string commonDataFileName_ = "SelectBoxCommon.bin";

	static inline Mesh planeMesh_;
	static inline std::unique_ptr<SHEngine::GPUBuffer> backGroundTexture_;

	//BackGroundにTextを合わせるためのオフセット
	static inline Vector2 textPositionOffset_ = {};
	static inline Vector2 textScaleOffset_ = {};
	static inline float lerpSpeed_ = 2.0f;
	static inline float putRatio_ = 0.2f;

	std::unique_ptr<SHEngine::BufferContainer> container_ = nullptr;

	std::unique_ptr<SHEngine::Text> text_ = nullptr;
	std::unique_ptr<SHEngine::Renderer> backGround_ = nullptr;

	SHEngine::GPUBuffer* matrixBuffer_ = nullptr;
	SHEngine::GPUBuffer* colorBuffer_ = nullptr;

	float t_ = 0.0f;	//座標変化用
	bool prevMouseInBox_ = false;
	bool isSelected_ = false;

	bool isUpdateSelected_ = false;

	Config config_ = {};
};
