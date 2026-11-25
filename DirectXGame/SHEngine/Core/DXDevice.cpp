#include "DXDevice.h"
#include <Render/PSO/Shelf/ShaderReflection.h>
#include <Utility/ConvertString.h>
#include <format>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "dxcompiler.lib")

using namespace SHEngine;

namespace {

	// @brief デフォルトのIncludeHandlerをラップし、読み込んだファイル(#include)を記録する
	// @note コンパイル中だけ使うスタック上のオブジェクトなので、参照カウントは管理しない
	class RecordingIncludeHandler final : public IDxcIncludeHandler {
	public:
		explicit RecordingIncludeHandler(IDxcIncludeHandler* inner) : inner_(inner) {}

		HRESULT STDMETHODCALLTYPE LoadSource(LPCWSTR pFilename, IDxcBlob** ppIncludeSource) override {
			HRESULT hr = inner_->LoadSource(pFilename, ppIncludeSource);
			if (SUCCEEDED(hr) && pFilename) {
				includes_.push_back(ConvertString(std::wstring(pFilename)));
			}
			return hr;
		}

		HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override {
			if (!ppvObject) {
				return E_POINTER;
			}
			if (riid == __uuidof(IDxcIncludeHandler) || riid == __uuidof(IUnknown)) {
				*ppvObject = static_cast<IDxcIncludeHandler*>(this);
				return S_OK;
			}
			*ppvObject = nullptr;
			return E_NOINTERFACE;
		}
		ULONG STDMETHODCALLTYPE AddRef() override { return 1; }
		ULONG STDMETHODCALLTYPE Release() override { return 1; }

		const std::vector<std::string>& GetIncludes() const { return includes_; }

	private:
		IDxcIncludeHandler* inner_ = nullptr;
		std::vector<std::string> includes_;
	};

}

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

    // GPUBufferの解放待ちキュー。BeginFrameでバッファ数フレーム前のフェンスを待つので、1フレーム余裕を持たせる
    deferredReleaseQueue_ = std::make_unique<DeferredReleaseQueue>(uint32_t(bufferCount + 1));
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

IDxcBlob* SHEngine::DXDevice::CompileShader(const std::string& filePath, ShaderType shaderType, ShaderReflectionData* outReflection, std::vector<std::string>* outIncludes) {
	std::wstring& profile = compileProfiles_[shaderType];
	std::wstring wFilePath = ConvertString(filePath);
    logger_->info("Begin CompileShader, path: {}", filePath);

    //hlslファイルを読む
    IDxcBlobEncoding* shaderSource = nullptr;
    HRESULT hr = dxcUtils_->LoadFile(wFilePath.c_str(), nullptr, &shaderSource);

	//読めなかった場合はエラー
	if (FAILED(hr)) {
		logger_->error("Failed to load shader file: {}", filePath);
        return nullptr;
	}

    //読み込んだファイルの内容を設定する
    DxcBuffer shaderSourceBuffer;
    shaderSourceBuffer.Ptr = shaderSource->GetBufferPointer();
    shaderSourceBuffer.Size = shaderSource->GetBufferSize();
    shaderSourceBuffer.Encoding = DXC_CP_UTF8;//utf8の文字コードであることを通知

#ifdef SH_DEBUG
    //Pix用
    LPCWSTR arguments[] = {
        wFilePath.c_str(),	            //コンパイル対象のhlslファイル名
        L"-E", L"main",                 //エントリーポイント
        L"-T", profile.c_str(),         //ShaderProfileの設定
        L"-Zi",                         //デバッグ情報を生成する
        L"-Qembed_debug",               //デバッグ用の情報をDXILに埋め込む
        L"-Od",                         //最適化を行わない
        L"-Zpr",                        //メモリレイアウトは行優先
        L"-enable-16bit-types"          //16bit型を有効化する
    };
#else
    //処理速度優先
    LPCWSTR arguments[] = {
        wFilePath.c_str(),	            //コンパイル対象のhlslファイル名
        L"-E", L"main",                 //エントリーポイント
        L"-T", profile.c_str(),         //ShaderProfileの設定
        L"-Qstrip_debug",               //デバッグ情報を削除する
        L"-Qstrip_reflect",             //リフレクション情報を削除する
        L"-O3",                         //最適化レベル3
        L"-Zpr",                        //メモリレイアウトは行優先
        L"-enable-16bit-types"          //16bit型を有効化する
    };
#endif
    //実際にShaderをコンパイルする
    RecordingIncludeHandler includeHandler(includeHandler_.Get());
    IDxcResult* shaderResult = nullptr;
    hr = dxcCompiler_->Compile(
        &shaderSourceBuffer,	//読み込んだファイル
        arguments,			    //コンパイルオプション
        _countof(arguments),	//コンパイルオプションの数
        &includeHandler,		//includeが含まれた諸々(読み込んだファイルを記録する)
        IID_PPV_ARGS(&shaderResult)		//コンパイル結果
    );
    //コンパイルエラーではなくdxcが起動できないなどの致命的な状況
    assert(SUCCEEDED(hr));

    if (outIncludes) {
        *outIncludes = includeHandler.GetIncludes();
    }

    //警告・エラーが出てたらログに出して止める
    IDxcBlobUtf8* shaderError = nullptr;
    shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), nullptr);
    if (shaderError != nullptr && shaderError->GetStringLength() != 0) {
        logger_->error(shaderError->GetStringPointer());
        shaderSource->Release();
        shaderError->Release();
        shaderResult->Release();
        return nullptr;
    }

    //コンパイル結果から実行用のバイナリ部分を取得
    IDxcBlob* shaderBlob = nullptr;
    hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
    assert(SUCCEEDED(hr));
    //成功したログを出す
    logger_->info("Compile Successd, path: {}", filePath);

    //リフレクション情報の取得(-Qstrip_reflectでも別出力として取得できる)
    if (outReflection) {
        *outReflection = {};
        Microsoft::WRL::ComPtr<IDxcBlob> reflectionBlob;
        hr = shaderResult->GetOutput(DXC_OUT_REFLECTION, IID_PPV_ARGS(&reflectionBlob), nullptr);
        if (SUCCEEDED(hr) && reflectionBlob && reflectionBlob->GetBufferSize() > 0) {
            DxcBuffer reflectionBuffer{};
            reflectionBuffer.Ptr = reflectionBlob->GetBufferPointer();
            reflectionBuffer.Size = reflectionBlob->GetBufferSize();
            reflectionBuffer.Encoding = 0;

            Microsoft::WRL::ComPtr<ID3D12ShaderReflection> reflection;
            hr = dxcUtils_->CreateReflection(&reflectionBuffer, IID_PPV_ARGS(&reflection));
            if (SUCCEEDED(hr) && ShaderReflector::Build(reflection.Get(), shaderType, *outReflection)) {
                logger_->debug("Shader reflection, path: {}\n{}", filePath, outReflection->ToString());
            } else {
                logger_->warn("Failed to create shader reflection, path: {}", filePath);
            }
        } else {
            logger_->warn("Shader reflection output not found, path: {}", filePath);
        }
    }

    //もう使わないリソースを開放
    shaderSource->Release();
    if (shaderError) {
        shaderError->Release();
    }
    shaderResult->Release();

    //実行用のバイナリを返却
    return shaderBlob;
}

const char* SHEngine::DXDevice::GetShaderCompileConfigName() {
	// CompileShaderの引数と対応させること(SH_DEBUGだけデバッグ情報付き・最適化なし)
#ifdef SH_DEBUG
	return "Debug";
#else
	return "Optimized";
#endif
}
