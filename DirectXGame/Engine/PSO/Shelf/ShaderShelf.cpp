#include "ShaderShelf.h"
#include <Utility/ConvertString.h>
#include <Utility/SearchFile.h>
#include <cassert>

namespace {

    IDxcBlob* CompileShader(
        //CompilerするShaderファイルへのパス
        const std::wstring& filePath,
        //Compilerに使用するprofile
        const wchar_t* profile,
        //初期化で生成したものを3つ
        IDxcUtils* dxcUtils,
        IDxcCompiler3* dxcCompiler,
        IDxcIncludeHandler* includeHandler,
        spdlog::logger* logger) {

        logger->info(ConvertString(std::format(L"Begin CompileShader, path: {}, profile: {}\n", filePath, profile)));

        //hlslファイルを読む
        IDxcBlobEncoding* shaderSource = nullptr;
        HRESULT hr = dxcUtils->LoadFile(filePath.c_str(), nullptr, &shaderSource);
        //読めなかったら止める
        assert(SUCCEEDED(hr));
        //読み込んだファイルの内容を設定する
        DxcBuffer shaderSourceBuffer;
        shaderSourceBuffer.Ptr = shaderSource->GetBufferPointer();
        shaderSourceBuffer.Size = shaderSource->GetBufferSize();
        shaderSourceBuffer.Encoding = DXC_CP_UTF8;//utf8の文字コードであることを通知

        LPCWSTR arguments[] = {
            filePath.c_str(),	//コンパイル対象のhlslファイル名
            L"-E", L"main",     //エントリーポイントの指定。基本的にmain以外にはしない
            L"-T", profile,    //ShaderProfileの設定
            L"-Zi", L"-Qembed_debug", //デバッグ用の情報を埋め込む
            L"-Od",     //最適化を行わない
            L"-Zpr",     //メモリレイアウトは行優先
        };
        //実際にShaderをコンパイルする
        IDxcResult* shaderResult = nullptr;
        hr = dxcCompiler->Compile(
            &shaderSourceBuffer,	//読み込んだファイル
            arguments,			    //コンパイルオプション
            _countof(arguments),	//コンパイルオプションの数
            includeHandler,		    //includeが含まれた諸々
            IID_PPV_ARGS(&shaderResult)		//コンパイル結果
        );
        //コンパイルエラーではなくdxcが起動できないなどの致命的な状況
        assert(SUCCEEDED(hr));

        //警告・エラーが出てたらログに出して止める
        IDxcBlobUtf8* shaderError = nullptr;
        shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), nullptr);
        if (shaderError != nullptr && shaderError->GetStringLength() != 0) {
            logger->error(shaderError->GetStringPointer());
            logger->flush();
            //警告・エラーが起きている状態なので止める
            assert(false);
        }

        //コンパイル結果から実行用のバイナリ部分を取得
        IDxcBlob* shaderBlob = nullptr;
        hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
        assert(SUCCEEDED(hr));
        //成功したログを出す
        logger->info(ConvertString(std::format(L"Compile Successd, path: {}, profile: {}\n", filePath, profile)));
        //もう使わないリソースを開放
        shaderSource->Release();
        shaderResult->Release();
        //実行用のバイナリを返却
        return shaderBlob;
    }

}

#pragma comment(lib, "dxcompiler.lib")

namespace fs = std::filesystem;

ShaderShelf::ShaderShelf() {

    //dxcCompilerを初期化
    HRESULT hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));
    assert(SUCCEEDED(hr));
    hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));
    assert(SUCCEEDED(hr));

    //includeに対応するための設定を行っておく
    hr = dxcUtils->CreateDefaultIncludeHandler(&includeHandler);
    assert(SUCCEEDED(hr));

	logger_ = Logger::getLogger("Core");

	compileVersions_[0] = L"vs_6_0"; // Vertex Shader
    compileVersions_[1] = L"ps_6_0"; // Pixel Shader
}

ShaderShelf::~ShaderShelf() {
    
}

void ShaderShelf::CompileAllShader() {

	std::vector<std::wstring> shaderNames;

    //shaderNameBufferを削除するためのスコープ
    {
        std::vector<std::string> shaderNameBuffer = SearchFiles(basePath_, ".hlsl");
        for (const auto& name : shaderNameBuffer) {
            shaderNames.push_back(ConvertString(name));
        }
    }

    for (const auto& sn : shaderNames) {
        if (sn.find(L".hlsli") != std::string::npos) {
            continue;
        }
        //VertexShaderだったら
        else if (sn.find(L"VS") != std::string::npos) {
			RegistShaderByteCode(sn, ShaderType::VERTEX_SHADER);
        }
		//PixelShaderだったら
        else if (sn.find(L"PS") != std::string::npos) {
			RegistShaderByteCode(sn, ShaderType::PIXEL_SHADER);
        } 
        //これ以降も同じように追加する


		// 登録してない、または名前が間違っているShaderは見つかり次第エラーを出す
        else {
            logger_->info(ConvertString(std::format(L"Unknown Shader Type: {}", sn)));
			throw std::runtime_error("Unknown shader type: " + ConvertString(sn));
        }
    }
}

std::list<D3D12_SHADER_BYTECODE> ShaderShelf::GetShaderBytecodes(ShaderType shaderType) {
    std::list<D3D12_SHADER_BYTECODE> ans;

    for (const auto& [name, bytecode] : shaderBytecodes_[static_cast<size_t>(shaderType)]) {
        ans.push_back(bytecode);
    }

	return ans;
}

std::list<std::string> ShaderShelf::GetShaderNames(ShaderType shaderType) {
    std::list<std::string> ans;

    for (const auto& [name, bytecode] : shaderBytecodes_[static_cast<size_t>(shaderType)]) {
        ans.push_back(name);
    }

    return ans;
}

D3D12_SHADER_BYTECODE ShaderShelf::GetShaderBytecode(ShaderType shaderType, std::string shaderName) {
    //登録されていなければCompileする
    if (shaderBytecodes_[static_cast<int>(shaderType)][shaderName].pShaderBytecode == D3D12_SHADER_BYTECODE().pShaderBytecode) {
		RegistShaderByteCode(ConvertString(shaderName), shaderType);
    }
	return shaderBytecodes_[static_cast<int>(shaderType)][shaderName];
}

void ShaderShelf::RegistShaderByteCode(std::wstring shaderName, ShaderType shaderType) {
    IDxcBlob* blobBuffer = CompileShader(ConvertString(basePath_.string()) + L"/" + shaderName, compileVersions_[int(shaderType)].c_str(),
        dxcUtils.Get(), dxcCompiler.Get(), includeHandler.Get(), logger_.get());

    shaderBlobs_.push_back(blobBuffer);

    shaderBytecodes_[int(shaderType)][ConvertString(shaderName)] = { blobBuffer->GetBufferPointer(), blobBuffer->GetBufferSize() };
}
