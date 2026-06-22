#pragma once
#include <Utility/SearchFile.h>

namespace Decorate {

	struct DecoPath {
		std::string name;
		std::string fullPath;
		std::vector<DecoPath> children;
	};

	class PathManager {
	public:

		PathManager();

		DecoPath GetDecoPath(const std::string& name);
		
		//BeginとEndは外で行うこと
		void DrawImGui();

		//folder内を再探索してDecoPathを更新する
		void UpdateDecoPath();

	private:

		void Update();

		void DrawDecoPath(const DecoPath& node);

		DecoPath root_;

	};
}
