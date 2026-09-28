#include"AudioManager.h"
#include "Data/MP3Data.h"
#include "Data/WAVData.h"
#include<cassert>
#include <filesystem>

#pragma comment(lib,"xaudio2.lib")

#pragma comment(lib, "Mf.lib")
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "Mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")

AudioManager* AudioManager::GetInstance() {
	static AudioManager instance;
	return &instance;
}

void AudioManager::Update() {
	for(const auto& data : audioData_) {
		data->Update();
	}
}

void AudioManager::Finalize() {
	xAudio2_.Reset();
	audioData_.clear();

	MFShutdown();

	CoUninitialize();
}

void AudioManager::Initialize() {

	// MediaFoundationの初期化
	MFStartup(MF_VERSION, MFSTARTUP_NOSOCKET);

	// XAudioエンジンのインスタンスを生成
	HRESULT result = XAudio2Create(&xAudio2_, 0, XAUDIO2_DEFAULT_PROCESSOR);
	assert(SUCCEEDED(result));

	// マスターボイスを生成
	result = xAudio2_->CreateMasteringVoice(&masterVoice_);
	assert(SUCCEEDED(result));
}

AudioData* AudioManager::GetData(std::string filePath) {
	//ファイルパスがAssetsから始まっているかどうか。
	std::string fixedFilePath = filePath;
	if (filePath.find("Assets/") != 0) {
		//Assets/Audio/を最初に追加する
		fixedFilePath = "Assets/Audio/" + filePath;
	}

	// 既に読み込まれているか検索
	for (const auto& data : audioData_) {
		if (data->GetName() == fixedFilePath) {
			return data.get();
		}
	}
	// 読み込まれていない場合は新規に読み込む
	std::unique_ptr<AudioData>& newData = audioData_.emplace_back();

	// ファイルの拡張子を取得
	std::filesystem::path path(fixedFilePath);
	std::string extension = path.extension().string();

	if (extension == ".mp3") {
		newData = std::make_unique<MP3Data>(xAudio2_.Get(), fixedFilePath);
	} else if (extension == ".wav") {
		newData = std::make_unique<WAVData>(xAudio2_.Get(), fixedFilePath);
	} else {
		assert(false && "Unsupported audio format");
		return nullptr;
	}

	return newData.get();
}

void AudioManager::SetVolume(float volume) {
	AudioData::SetOverallVolume(volume);
}
