#pragma once
#include <SHEngine.h>
#include "DecoObjectRender.h"

namespace Decorate {

	//オブジェクトの追加と描画を行うクラス。データ管理もごく簡易的に行うが、基本はControllerからの命令で動く
	class ObjManager {
	public:

		ObjManager(SHEngine::Screen::Display* display, SHEngine::Engine* engine);

		void Update(Camera* camera);

		void Draw(DCC* dcc);

		void SetTransform(uint32_t id, const Transform& transform);
		Matrix4x4 GetTransform(uint32_t id) const;

	private:

		std::string currentPath_ = "";

		SHEngine::Screen::Display* display_ = nullptr;
		SHEngine::Engine* engine_ = nullptr;

		struct RenderInfo {
			std::vector<uint32_t> ids;
			std::vector<Matrix4x4> transforms;
		};
		std::vector<RenderInfo> renderInfos_;
		std::vector<std::unique_ptr<ObjRenderer>> renderers_;

		uint32_t nextID_ = 1;
	};

}
