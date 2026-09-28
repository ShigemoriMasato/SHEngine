#pragma once
#include <Utility/SearchFile.h>
#include <Assets/Texture/TextureManager.h>
#include "DecoDataManager.h"

namespace Decorate {

	struct DecoPath {
		std::string name;
		std::string fullPath;
		std::vector<DecoPath> children;
	};

	class PathManager {
	public:

		PathManager(SHEngine::TextureManager* textureManager, DataManager* dataManager);

		DecoPath GetDecoPath(const std::string& name);
		
		//BeginとEndは外で行うこと
		void DrawImGui();

		//folder内を再探索してDecoPathを更新する
		void UpdateDecoPath();

	private:

		void Update();

		SHEngine::TextureManager* textureManager_;
		DataManager* dataManager_;

		DecoPath root_;

		std::vector<std::string> currentPath_;

		int folder_ = 0;
		int model_ = 0;
	};
}
