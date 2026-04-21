#pragma once
#include <cstdint>

namespace SHEngine {

	class FrameCounter {
	public:

		void Initialize();
		void Update();

	private:

		uint32_t frame_ = 0;

	};

}
