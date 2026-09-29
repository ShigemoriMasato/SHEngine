#pragma once
#include <SandSimulator/Data/SandData.h>
#include <Assets/Model/ModelData.h>
#include <Render/Renderer.h>

namespace Sand {
	class DebugGrid {
	public:

		DebugGrid(Grid* gridData, ModelData* cubeModel, SHEngine::BufferContainer* container);

		void Initialize();
		void Update();
		void Draw(const DCC* dcc);

	private:

		std::unique_ptr<SHEngine::Renderer> renderer_ = nullptr;

		Grid* gridData_ = nullptr;
		SHEngine::GPUBuffer* drawDataBuffer_ = nullptr;

		struct DebugGridData {
			Vector3 upperColor;
			float alpha;
			Vector3 lowerColor;
			float outlineWidth;
		}drawData_;

	private: //編集目録

		float alpha_ = 0.3f;
		Vector3 upperColor_ = { 1.0f, 0.0f, 0.0f };
		Vector3 lowerColor_ = { 1.0f, 1.0f, 1.0f };

		float outlineWidth_ = 0.01f;

	};
}
