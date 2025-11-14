#pragma once
#include "Core/Device/DXDevice.h"
#include <Utility/LeakChecker.h>
#include <Core/View/SRVManager.h>
#include <Core/View/RTVManager.h>
#include <Core/View/DSVManager.h>
#include <Assets/Texture/TextureManager.h>
#include <Display/SwapChain.h>
#include <memory>

class SHEngine {
public:

	SHEngine();
	~SHEngine();

	void Initialize();

	bool IsLoop();

	void Update();

	void PreDraw();
	void EndFrame(ID3D12CommandList** commandLists);

private:

	//LeakChecker
	D3DResourceLeakChecker leakChecker_;

	std::unique_ptr<SRVManager> srvManager_;
	std::unique_ptr<RTVManager> rtvManager_;
	std::unique_ptr<DSVManager> dsvManager_;

private:

	std::unique_ptr<DXDevice> dxDevice_;

	std::unique_ptr<TextureManager> textureManager_;

	MSG msg_;

};
