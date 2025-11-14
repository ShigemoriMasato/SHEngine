#include "SHEngine.h"

SHEngine::SHEngine() {
	dxDevice_ = std::make_unique<DXDevice>();
	dxDevice_->Initialize();

	srvManager_ = std::make_unique<SRVManager>(dxDevice_.get(), 2048);
	rtvManager_ = std::make_unique<RTVManager>(dxDevice_.get(), 128);
	dsvManager_ = std::make_unique<DSVManager>(dxDevice_.get(), 128);

	textureManager_ = std::make_unique<TextureManager>();

	Display::StaticInitialize(dxDevice_->GetDevice(), rtvManager_.get(), dsvManager_.get());
}

SHEngine::~SHEngine() {
	
}

void SHEngine::Initialize() {
	textureManager_->Initialize(dxDevice_.get(), srvManager_.get());
	SwapChain::StaticInitialize(dxDevice_.get(), textureManager_.get());
}

bool SHEngine::IsLoop() {
	while (msg_.message != WM_QUIT) {
		//メッセージがあれば処理する
		if (PeekMessage(&msg_, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg_);
			DispatchMessage(&msg_);
		} else {
			//メッセージがなければ処理を始める
			return true;
		}
	}

	return false;
}

void SHEngine::Update() {
}

void SHEngine::PreDraw() {
	dxDevice_->WaitSignal();
}

void SHEngine::EndFrame(ID3D12CommandList** commandLists) {
	dxDevice_->Execute(commandLists);
	dxDevice_->SendSignal();
}
