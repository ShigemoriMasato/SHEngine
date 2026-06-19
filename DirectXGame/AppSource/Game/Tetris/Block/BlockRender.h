#pragma once
#include <Camera/Camera.h>
#include <Game/Tetris/Data.h>
#include <Render/Renderer.h>
#include <Utility/DataStructures.h>
#include "Delete/DeleteEffect.h"

class BlockRender {
public:

	BlockRender() = default;
	~BlockRender();

	void Initialize(uint32_t fieldWidth, uint32_t fieldHeight, Camera* camera, const SHEngine::DrawData& drawData, SHEngine::TextureData* ddsTexture);

	void Update(float deltaTime);

	void SetStageData(std::vector<std::vector<int>> fieldData, const MovableMino& mino);
	//4*1まで
	void SetHoldMino(std::vector<std::pair<int, int>> blockPos, int colorID);
	//4*5まで
	void SetNextMino(std::vector<std::pair<int, int>> blockPos, int colorID);

	void BeginDeleteEffect(std::vector<int> fillLines, std::vector<std::vector<int>> deletedField);

	void Draw(DCC* cmdObj);
	void DrawImGui();

	bool GetIsEffecting() const { return isDeleting_; }

private:

	void SetBlock(int x, int y, int configIndex);
	void SetBlock(std::vector<std::vector<int>> allConfigIndices, MovableMino movableMino);

	Logger logger_;

	struct ColorMap {
		Vector4 color;
		Vector4 outlineColor;
	};
	std::vector<ColorMap> colorMap_{};
	Camera* camera_ = nullptr;

	//Field->Wall->Hold->Next
	std::unique_ptr<SHEngine::BufferContainer> container_;
	std::unique_ptr<SHEngine::Renderer> blockRenderer_;
	SHEngine::GPUBuffer* vsBuffer_;
	SHEngine::GPUBuffer* colorMapBuffer_;
	SHEngine::GPUBuffer* psBuffer_;
	std::vector<Transform> blockTransforms_{};


	struct VSData {
		Matrix4x4 world = Matrix4x4::Identity();
		Matrix4x4 wvp = Matrix4x4::Identity();
		uint32_t colorID;
	};
	struct PSData {
		Vector3 cameraPos;
		float strength = 0.5f;
	};
	int vertexDataIndex_ = -1;
	std::vector<VSData> vsData_{};
	PSData psData_{};

	//表示するフィールドサイズ
	uint32_t fieldWidth_ = 0;
	uint32_t fieldHeight_ = 0;

private://DeleteEffect

	std::unique_ptr<DeleteEffect> deleteEffect_ = nullptr;
	std::vector<std::vector<int>> deletedField_{};
	bool isDeleting_ = false;
	std::vector<int> deletingLines_{};

private://Binary保存

	void Save();
	void Load();

	std::unique_ptr<BinaryManager> binaryManager_ = nullptr;
	static inline const std::string fileName_ = "TetrisBlockRenderData.sg";
	
	Vector3 holdBasePosition_ = Vector3(-11.0f, 6.0f, 0.0f);
	Vector3 nextBasePosition_ = Vector3(11.0f, 6.0f, 0.0f);
	Vector3 nextGap_ = Vector3(0.0f, -4.0f, 0.0f);

private://ImGui
	int colorMapEditID_ = 0;
};
