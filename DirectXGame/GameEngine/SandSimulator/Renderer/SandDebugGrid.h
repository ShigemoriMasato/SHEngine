#pragma once
#include <SandSimulator/Data/SandData.h>
#include <Assets/Model/ModelData.h>
#include <Render/Renderer.h>

namespace Sand {
	class DebugGrid {
	public:

		DebugGrid(Grid* gridData, ModelData* cubeModel, SHEngine::BufferContainer* container);

		void Initialize();
		void Update(Camera* camera);
		void Draw(DCC* dcc);
		void DrawImGui();

		void OpenMaterialEditWindow() { openOrder_ = true; }

		void Save(BinaryManager& binManager) const;
		void Load(BinaryManager& binManager);

	private:

		std::unique_ptr<SHEngine::Renderer> renderer_ = nullptr;

		Grid* gridData_ = nullptr;
		SHEngine::GPUBuffer* matrixBuffer_ = nullptr;
		SHEngine::GPUBuffer* materialBuffer_ = nullptr;

		struct MaterialData {
			Vector3 upperColor{};
			float alpha = 0.3f;
			Vector3 lowerColor{};
			float outlineWidth = 0.01f;
			Vector4 outlineColor;
		}materialData_;

		struct MatrixData {
			Matrix4x4 vpMatrix;
		};

	private: //編集目録

		bool isVisible_ = true;
		bool openOrder_ = false;
		bool isMaterialWindowOpened_ = false;

	};
}
