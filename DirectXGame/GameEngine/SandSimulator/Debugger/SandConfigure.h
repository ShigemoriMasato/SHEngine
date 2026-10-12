#pragma once
#include <string>
#include <unordered_map>
#include <functional>

namespace Sand {

	class Configure {
	public:

		//Begin,EndをのぞいたImGuiの関数
		void Add(std::function<void()> drawImGui, std::string name);

		void DrawImGui();

	private:

		struct Conf {
			bool isOpen = false;
		};

		std::unordered_map<std::string, std::function<void()>> drawImGuiList_;
		std::unordered_map<std::string, Conf> confList_;

		int height_ = 0;
	};

}
