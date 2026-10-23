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

	void Initialize(uint32_t fieldWidth, uint32_t fieldHeight, Camera* camera, const Mesh& cubeMesh, SHEngine::TextureData* ddsTexture);

	void Update(float deltaTime);

	void SetStageData(std::vector<std::vector<int>> fieldData, const MovableMino& mino);
	//4*1まで
	void SetHoldMino(std::vector<std::pair<int, int>> blockPos, int colorID);
	//4*5まで
	void SetNextMino(std::vector<std::pair<int, int>> blockPos, int colorID);

	void SetParentMatrix(Matrix4x4 mat) { parentMatrix_ = mat; }

	void BeginDeleteEffect(std::vector<int> fillLines, std::vector<std::vector<int>> deletedField);

	void Draw(DCC* cmdObj);
	void DrawImGui();

	bool GetIsEffecting() const { return isDeleting_; }

	std::vector<Transform> DeleteLinesTransform() const;

	Vector3 BoxSize() const { return Vector3(1.0f, 1.0f, 1.0f); };
	Vector3 GetLowerLeftPos() const { return lowerLeftPos_; }

private:

	void SetBlock(int x, int y, int configIndex);
	void SetBlock(std::vector<std::vector<int>> allConfigIndices, MovableMino movableMino);

	Logger logger_;

	struct ColorMap {
		Vector4 color;
		Vector4 outlineColor;
	};
	struct VSData {
		Matrix4x4 world = Matrix4x4::Identity();
		Matrix4x4 wvp = Matrix4x4::Identity();
	};
	struct MaterialData {
		Vector4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
		float intensity = 0.0f;
		uint32_t colorID;
	};
	std::vector<ColorMap> colorMap_{};
	std::vector<VSData> vsData_{};
	std::vector<MaterialData> materialData_{};

	SHEngine::GPUBuffer* vsBuffer_;
	SHEngine::GPUBuffer* colorMapBuffer_;
	SHEngine::GPUBuffer* materialBuffer_;

	Matrix4x4 parentMatrix_ = Matrix4x4::Identity();

	//Field->Wall->Hold->Next
	std::unique_ptr<SHEngine::BufferContainer> container_;
	std::unique_ptr<SHEngine::Renderer> blockRenderer_;

	Camera* camera_ = nullptr;
	std::vector<Transform> blockTransforms_{};

	int vertexDataIndex_ = -1;

	//表示するフィールドサイズ
	uint32_t fieldWidth_ = 0;
	uint32_t fieldHeight_ = 0;

	Vector3 lowerLeftPos_;

private://DeleteEffect

	std::unique_ptr<DeleteEffect> deleteEffect_ = nullptr;
	std::vector<std::vector<int>> deletedField_{};
	bool isDeleting_ = false;
	std::vector<int> deletingLines_{};

private://Binary保存

	void Save();
	void Load();

	std::unique_ptr<BinaryManager> binaryManager_ = nullptr;
	static inline const std::string fileName_ = "TetrisBlockRenderData.bin";
	
	Vector3 holdBasePosition_ = Vector3(-11.0f, 6.0f, 0.0f);
	Vector3 nextBasePosition_ = Vector3(11.0f, 6.0f, 0.0f);
	Vector3 nextGap_ = Vector3(0.0f, -4.0f, 0.0f);

private://ImGui
	int colorMapEditID_ = 0;
};
