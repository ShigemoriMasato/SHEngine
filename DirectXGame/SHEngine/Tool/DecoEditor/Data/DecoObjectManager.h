#pragma once
#include <SHEngine.h>
#include "DecoDataManager.h"
#include <Tool/ModelDrawer/ModelDrawer.h>

namespace Decorate {

	//オブジェクトの追加と描画を行うクラス。データ管理もごく簡易的に行うが、基本はControllerからの命令で動く
	class ObjManager {
	public:

		ObjManager(SHEngine::Screen::Display* display, SHEngine::Engine* engine, DataManager* dataManager);

		void Update(Camera* camera);

		void Draw(DCC* dcc);
		void NormalDraw(DCC* dcc);

		void SetCamera(Camera* camera);

	private:

		DataManager* dataManager_ = nullptr;

		std::string currentPath_ = "";

		SHEngine::Screen::Display* display_ = nullptr;
		SHEngine::Engine* engine_ = nullptr;

		struct RenderInfo {
			std::vector<uint32_t> ids;
			std::vector<Matrix4x4> transforms;
		};
		std::vector<RenderInfo> renderInfos_;
		std::unordered_map<std::string, std::unique_ptr<ModelDrawer>> renderers_;
	};

}
