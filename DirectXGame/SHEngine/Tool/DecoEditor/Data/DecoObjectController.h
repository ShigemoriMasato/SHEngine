#pragma once
#include <Render/Screen/Display.h>
#include <Compute/ComputeObject.h>
#include "DecoObjectManager.h"
#include "DecoDataManager.h"

namespace Decorate {

	//オブジェクトの選択、編集を行い、マネージャに伝えるクラス
	class ObjController {
	public:

		//ReadBack用のリソースを作成するために、DXDeviceを渡す必要がある
		static void Initialize(SHEngine::DXDevice* device) { device_ = device; }

		ObjController(SHEngine::Screen::Display* display, SHEngine::Engine* engine, DataManager* dataManager);

		void Update(Camera* camera, DCC* dcc);

	private:

		void GetIDFromGPU(DCC* dcc);

		void EditObject(Camera* camera);

		static inline SHEngine::DXDevice* device_ = nullptr;

		DataManager* dataManager_ = nullptr;
		SHEngine::Screen::Display* display_ = nullptr;
		SHEngine::Engine* engine_ = nullptr;

		struct Resource {
			Microsoft::WRL::ComPtr<ID3D12Resource> res;
		};
		std::vector<Resource> readBacks_;

		std::unique_ptr<SHEngine::BufferContainer> container_ = nullptr;
		std::unique_ptr<SHEngine::ComputeObject> idGetter_ = nullptr;

		SHEngine::GPUBuffer* cursorBuffer_ = nullptr;
		SHEngine::GPUBuffer* ansBuffer_ = nullptr;

		uint32_t selectedID_ = 0;

		bool click_ = false;
		bool preClick_ = false;

		bool isImGuizmoActive_ = false;
	};

}
