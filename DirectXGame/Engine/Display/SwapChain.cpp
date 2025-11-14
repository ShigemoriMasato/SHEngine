#include "SwapChain.h"

DXDevice* SwapChain::dxDevice_ = nullptr;
TextureManager* SwapChain::textureManager_ = nullptr;

void SwapChain::StaticInitialize(DXDevice* dxDevice, TextureManager* textureManager) {
	textureManager_ = textureManager;
	dxDevice_ = dxDevice;
}

void SwapChain::CreateCommandObject() {
    if (!logger_) {
        logger_ = Logger::getLogger("Core");
    }

    for (int i = 0; i < 2; ++i) {
        //CommandAllocator
        HRESULT hr = dxDevice_->GetDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator_[i]));
        //コマンドアロケータの生成がうまくいかなかったので起動できない
        assert(SUCCEEDED(hr));
        logger_->info("Complete create CommandAllocator\n");

        //CommandList
        hr = dxDevice_->GetDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator_[i].Get(), nullptr, IID_PPV_ARGS(&commandList_[i]));
        //コマンドリストの生成がうまくいかなかったので起動できない
        assert(SUCCEEDED(hr));
        logger_->info("Complete create CommandList\n");
    }
}

void SwapChain::Initialize(WindowsApp* createdApp, uint32_t color) {
    auto windowSize = createdApp->GetWindowSize();
    int offset[2] = {};

    for (int i = 0; i < 2; ++i) {
        auto display = std::make_unique<Display>();
        offset[i] = textureManager_->CreateWindowTexture(windowSize.first, windowSize.second, color, true);
        display->Initialize(textureManager_->GetTextureData(offset[i]), color);
        display_[i] = std::move(display);
    }

    //スワップチェーンを生成する
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
    swapChainDesc.Width = windowSize.first;	                        //ウィンドウ幅
    swapChainDesc.Height = windowSize.second;	                    //ウィンドウ高さ
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;      	    //色の形式
    swapChainDesc.SampleDesc.Count = 1;	                            //マルチサンプルしない
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;	//描画のターゲットとして利用する
    swapChainDesc.BufferCount = 2;	//ダブルバッファ
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;   	//モニタに映したら捨てる

    //コマンドキュー、ウィンドウハンドル、設定を渡して生成する
    HRESULT hr = dxDevice_->GetDxgiFactory()->CreateSwapChainForHwnd(
        dxDevice_->GetCommandQueue(),		        		//コマンドキュー
        createdApp->GetHwnd(),			            //ウィンドウハンドル
        &swapChainDesc,	        		        //設定
        nullptr,		    	    		    //モニタの設定
        nullptr,			    		        //出力の設定
        reinterpret_cast<IDXGISwapChain1**>(swapChain_.GetAddressOf()));	//スワップチェーンのポインタ

    assert(SUCCEEDED(hr));

    logger_->info("Complete create SwapChain\n");

    for (int i = 0; i < 2; ++i) {
        Microsoft::WRL::ComPtr<ID3D12Resource> resourcePtr = textureManager_->GetTextureResource(offset[i]);
        swapChain_->GetBuffer(i, IID_PPV_ARGS(&resourcePtr));
    }

    index_ = swapChain_->GetCurrentBackBufferIndex();

	ID3D12CommandList* lists[] = { commandList_[0].Get(), commandList_[1].Get() };
    dxDevice_->Execute(lists);
    dxDevice_->SendSignal();
}

void SwapChain::SwitchRenderTarget(bool isClear) {
	display_[index_]->DrawReady(commandList_[index_].Get(), true);
}

void SwapChain::SwitchOffScreen(Display* display) {
    display->DrawReady(commandList_[index_].Get(), false);
}

void SwapChain::BeginCommand() {
}

void SwapChain::Present() {
    HRESULT hr = swapChain_->Present(1, 0);
    assert(SUCCEEDED(hr));
    //次のバッファへ
	index_ = swapChain_->GetCurrentBackBufferIndex();
}
