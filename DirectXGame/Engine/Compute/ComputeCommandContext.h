#pragma once
#include <Core/Command/ICommandContext.h>

namespace SHEngine {

	class ComputeCommandContext : public ICommandContext {
	public:

		void Initialize(DXDevice* device, int initCmdObjNum = 2);

	private:



	};

}
