#pragma once
#include <Render/Renderer.h>
#include <Camera/Camera.h>

namespace Decorate {

	class ObjRenderer {
	public:

		ObjRenderer(const SHEngine::DrawData& drawData, int textureIndex);

		void Update(Camera* camera);
		void Draw(DCC* dcc);

		void SetObjInfo(const std::vector<Matrix4x4>& transforms, const std::vector<uint32_t>& ids);

	private:

		static inline const uint32_t maxSize_ = 128;

		std::unique_ptr<SHEngine::BufferContainer> container_ = nullptr;
		std::unique_ptr<SHEngine::Renderer> renderer_ = nullptr;

		SHEngine::GPUBuffer* cameraBuffer_ = nullptr;
		SHEngine::GPUBuffer* worldBuffer_ = nullptr;
		SHEngine::GPUBuffer* idBuffer_ = nullptr;
	};

}