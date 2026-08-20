#pragma once
#include <Game/Tetris/Tetris.h>
#include <Game/Effect/Polygon/PolygonEmitter.h>

class DeleteLineMeshEffect {
public:

	DeleteLineMeshEffect() = default;

	void Initialize(Tetris* tetris, PolygonEmitter* emitter, const Mesh& cubeMesh);
	void Update(float deltaTime);

	void DrawImGui();

	void SetParentMatrix(Matrix4x4 mat) { parentMatrix_ = mat; }

	void Save(BinaryManager& bin) const;
	void Load(BinaryManager& bin);

private:

	Tetris* tetris_ = nullptr;
	PolygonEmitter* emitter_ = nullptr;

	std::vector<PolygonEmitter::Config> configs_;

	PolygonEmitter::Config emitConfig_{};
	PolygonEmitter::Config zeroConfig_{};

	Matrix4x4 parentMatrix_ = Matrix4x4::Identity();

	bool debugDeleteLine_ = false;
};
