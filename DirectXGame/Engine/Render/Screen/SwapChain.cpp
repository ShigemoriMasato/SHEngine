#include "SwapChain.h"
#include <Utility/DirectUtilFuncs.h>

void SHEngine::Screen::SwapChain::Initialize(TextureManager* textureManager, DirectCommandContext* directContext, uint32_t clearColor, std::unique_ptr<WindowsAPI> window) {
    logger_ = getLogger("Engine");

    logger_->info("=== Start create SwapChain ===");

    HWND hwnd = window->GetHwnd();
    auto [width, height] = window->GetWindowSize();

    //スワップチェーンを生成する
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
    swapChainDesc.Width = width;	                        //ウィンドウ幅
    swapChainDesc.Height = height;	                        //ウィンドウ高さ
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;      	    //色の形式
    swapChainDesc.SampleDesc.Count = 1;	                            //マルチサンプルしない
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;	//描画のターゲットとして利用する
    swapChainDesc.BufferCount = bufferCount_;	//バッファの数
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;   	//モニタに映したら捨てる

    auto commandQueue = directContext->GetCommandQueue();

    //コマンドキュー、ウィンドウハンドル、設定を渡して生成する
    HRESULT hr = device_->GetDxgiFactory()->CreateSwapChainForHwnd(
        commandQueue,		        		                                //コマンドキュー
        hwnd,			                                                    //ウィンドウハンドル
        &swapChainDesc,	        		                                    //設定
        nullptr,		    	    		                                //モニタの設定
        nullptr,			    		                                    //出力の設定
        reinterpret_cast<IDXGISwapChain1**>(swapChain_.GetAddressOf()));	//スワップチェーンのポインタ
    assert(SUCCEEDED(hr));

    logger_->info("Complete create SwapChain");

    ID3D12Resource* swapChainResources;
	displays_.resize(bufferCount_);
    for (int i = 0; i < bufferCount_; i++) {
        //SwapChainからResourceを取得する
        hr = swapChain_->GetBuffer(i, IID_PPV_ARGS(&swapChainResources));
        assert(SUCCEEDED(hr));

        //Displayを初期化する
        displays_[i] = std::make_unique<Display>();
		displays_[i]->Initialize(textureManager, swapChainResources, clearColor);
    }

    logger_->info("SwapChain Info: Width : {} Height : {} ClearColor : {:08X}", width, height, clearColor);
    logger_->info("=== Complete create SwapChain ===");

	currentBufferIndex_ = swapChain_->GetCurrentBackBufferIndex();

	window_ = std::move(window);
}

void SHEngine::Screen::SwapChain::Clear(Command::Object* cmdObject) {
	auto cmdList = cmdObject->GetCommandList();
    float color[4];
    Vector4 clearColor = displays_[currentBufferIndex_]->GetTextureData()->GetClearColor();
    color[0] = clearColor.x;
    color[1] = clearColor.y;
    color[2] = clearColor.z;
    color[3] = clearColor.w;
	cmdList->ClearRenderTargetView(GetRTVHandle(), color, 0, nullptr);
}

void SHEngine::Screen::SwapChain::Present() {
	swapChain_->Present(1, 0);
    currentBufferIndex_ = swapChain_->GetCurrentBackBufferIndex();
}

void SHEngine::Screen::SwapChain::ToRenderTarget(Command::Object* cmdObject) {
	displays_[currentBufferIndex_]->ToRenderTarget(cmdObject);
}

void SHEngine::Screen::SwapChain::ToPresent(Command::Object* cmdObject) {
	displays_[currentBufferIndex_]->ToPresent(cmdObject);
}

void SHEngine::Screen::SwapChain::ToTexture(Command::Object* cmdObject) {
	displays_[currentBufferIndex_]->ToTexture(cmdObject);
}
