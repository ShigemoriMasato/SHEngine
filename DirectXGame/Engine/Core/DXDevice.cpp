#include "DXDevice.h"
#include <Utility/ConvertString.h>
#include <format>
#include <Utility/ConvertString.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "dxcompiler.lib")

using namespace SHEngine;

DXDevice::~DXDevice() {
}

void DXDevice::Initialize() {
	logger_ = GetLogger("Engine");
	logger_->info("=== DXDevice ===");

#if SH_DEBUG

    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController_)))) {
        //デバッグレイヤーを有効化する
        debugController_->EnableDebugLayer();
        //さらにGPU側でもチェックを行うようにする
        debugController_->SetEnableGPUBasedValidation(TRUE);
    }

    Microsoft::WRL::ComPtr<ID3D12Debug1> debugController1;
    if (SUCCEEDED(debugController_->QueryInterface(IID_PPV_ARGS(&debugController1)))) {
        debugController1->SetEnableGPUBasedValidation(TRUE);
        debugController1->SetEnableSynchronizedCommandQueueValidation(TRUE);
    }

#endif

    // DXGIFactory
    //関数が成功したかどうかをSUCCEEDEDマクロで判定する
    HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory_));
    assert(SUCCEEDED(hr));

    // Adapter作成
    //一番いいアダプタを頼む
    for (UINT i = 0; dxgiFactory_->EnumAdapterByGpuPreference(i,
        DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&useAdapter_)) != DXGI_ERROR_NOT_FOUND; ++i) {
        //アダプタの情報を取得する
        DXGI_ADAPTER_DESC3 adapterDesc{};
        hr = useAdapter_->GetDesc3(&adapterDesc);
        //取得できなければエラー
        assert(SUCCEEDED(hr));
        //ソフトウェアアダプタでなければ採用
        if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)) {
            //採用したアダプタの情報をログに出力
            logger_->info(ConvertString(std::format(L"Use Adapter:{}", adapterDesc.Description)));

            break;
        }
        //ソフトウェアアダプタの場合はなかったことに
        useAdapter_ = nullptr;
    }

    //適切なアダプタが見つからなかった場合は起動できない
    assert(useAdapter_ != nullptr);

    // Device作成

    //機能レベルとログ出力用の文字列
    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_12_2,
        D3D_FEATURE_LEVEL_12_1,
        D3D_FEATURE_LEVEL_12_0
    };
    std::string featureLevelStrings[] = { "12.2","12.1","12.0" };
    //高い順に生成できるか試していく
    for (size_t i = 0; i < _countof(featureLevels); ++i) {
        //採用したアダプタでデバイスを生成
        hr = D3D12CreateDevice(useAdapter_.Get(), featureLevels[i], IID_PPV_ARGS(&device_));
        //指定した機能レベルでデバイスが生成できたか確認
        if (SUCCEEDED(hr)) {
            //生成できたのでログを出力してループを抜ける
            logger_->info(std::format("FeatureLevel:{}", featureLevelStrings[i]));
            break;
        }
    }

    //デバイスの生成が上手くいかなかったので起動できない
    assert(device_ != nullptr);
    logger_->info("Complete create D3D12Device");

    // DescriptorSizeの取得

    uint32_t descriptorSizeSRV = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    uint32_t descriptorSizeRTV_ = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    uint32_t descriptorSizeDSV = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	//各種DescriptorHeapManagerの生成
    srvManager_ = std::make_unique<SRVManager>(device_.Get(), descriptorSizeSRV, 4096);
    rtvManager_ =  std::make_unique<RTVManager>(device_.Get(), descriptorSizeRTV_, 128);
    dsvManager_ =  std::make_unique<DSVManager>(device_.Get(), descriptorSizeDSV, 128);
	logger_->info("Complete create DescriptorHeapManagers");

#ifdef SH_DEBUG

    Microsoft::WRL::ComPtr<ID3D12InfoQueue> infoQueue = nullptr;
    if (SUCCEEDED(device_->QueryInterface(IID_PPV_ARGS(&infoQueue)))) {
        //やばいエラーの時に止まる
        infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
        //エラーの時に止まる
        infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
        //警告の時に止まる(これをコメントアウトすると解放していないObjectを特定できる(?))
        infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);

        //抑制するメッセージのID
        D3D12_MESSAGE_ID denyIds[] = {
            //windows11でのDXGIデバッグレイヤーとDX12デバッグレイヤーの相互作用バグによるエラーメッセージ
            //https://stackoverflow.com/questions/69805245/directx-12-application-is-crashing-in-windows-11
            D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE
        };
        //抑制するレベル
        D3D12_MESSAGE_SEVERITY severities[] = { D3D12_MESSAGE_SEVERITY_INFO };
        D3D12_INFO_QUEUE_FILTER filter{};
        filter.DenyList.NumIDs = _countof(denyIds);
        filter.DenyList.pIDList = denyIds;
        filter.DenyList.NumSeverities = _countof(severities);
        filter.DenyList.pSeverityList = severities;
        //指定したメッセージの行事を抑制する
        infoQueue->PushStorageFilter(&filter);
    }

	logger_->info("Debug layer is enabled.");

#endif

    //dxcCompilerを初期化
    hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils_));
    assert(SUCCEEDED(hr));
    hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler_));
    assert(SUCCEEDED(hr));

    //includeに対応するための設定を行っておく
    hr = dxcUtils_->CreateDefaultIncludeHandler(&includeHandler_);
    assert(SUCCEEDED(hr));
}

IDxcBlob* SHEngine::DXDevice::CompileShader(const std::string& filePath, ShaderType shaderType) {
	std::wstring profile = compileProfiles_[shaderType];
	std::wstring wFilePath = ConvertString(filePath);
    logger_->info("Begin CompileShader, path: {}", filePath);

    //hlslファイルを読む
    IDxcBlobEncoding* shaderSource = nullptr;
    HRESULT hr = dxcUtils_->LoadFile(wFilePath.c_str(), nullptr, &shaderSource);
    //読めなかったら止める
    assert(SUCCEEDED(hr));
    //読み込んだファイルの内容を設定する
    DxcBuffer shaderSourceBuffer;
    shaderSourceBuffer.Ptr = shaderSource->GetBufferPointer();
    shaderSourceBuffer.Size = shaderSource->GetBufferSize();
    shaderSourceBuffer.Encoding = DXC_CP_UTF8;//utf8の文字コードであることを通知

    LPCWSTR arguments[] = {
        wFilePath.c_str(),	//コンパイル対象のhlslファイル名
        L"-E", L"main",     //エントリーポイントの指定。基本的にmain以外にはしない
        L"-T", profile.c_str(),    //ShaderProfileの設定
        L"-Zi", L"-Qembed_debug", //デバッグ用の情報を埋め込む
        L"-Od",     //最適化を行わない
        L"-Zpr",     //メモリレイアウトは行優先
    };
    //実際にShaderをコンパイルする
    IDxcResult* shaderResult = nullptr;
    hr = dxcCompiler_->Compile(
        &shaderSourceBuffer,	//読み込んだファイル
        arguments,			    //コンパイルオプション
        _countof(arguments),	//コンパイルオプションの数
        includeHandler_.Get(),		    //includeが含まれた諸々
        IID_PPV_ARGS(&shaderResult)		//コンパイル結果
    );
    //コンパイルエラーではなくdxcが起動できないなどの致命的な状況
    assert(SUCCEEDED(hr));

    //警告・エラーが出てたらログに出して止める
    IDxcBlobUtf8* shaderError = nullptr;
    shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), nullptr);
    if (shaderError != nullptr && shaderError->GetStringLength() != 0) {
        logger_->error(shaderError->GetStringPointer());
        //警告・エラーが起きている状態なので止める
        assert(false);
    }

    //コンパイル結果から実行用のバイナリ部分を取得
    IDxcBlob* shaderBlob = nullptr;
    hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
    assert(SUCCEEDED(hr));
    //成功したログを出す
    logger_->info("Compile Successd, path: {}", filePath);

    //もう使わないリソースを開放
    shaderSource->Release();
    shaderResult->Release();

    //実行用のバイナリを返却
    return shaderBlob;
}
