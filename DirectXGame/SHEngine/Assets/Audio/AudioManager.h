#pragma once
#include<xaudio2.h>
#include<fstream>
#include <wrl.h>
#include<vector>
#include <unordered_map>
#include "Data/AudioData.h"

#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>

class AudioManager {
public:

	// シングルトンインスタンスの取得
	static AudioManager* GetInstance();

	// コピーコンストラクタと代入演算子を無効化（シングルトン保証のため）
	AudioManager(const AudioManager&) = delete;
	AudioManager& operator=(const AudioManager&) = delete;

	void Initialize();

	void Update();

	void Finalize();

	AudioData* GetData(std::string filePath);

	void SetVolume(float volume);

private:

	AudioManager() = default;

	Microsoft::WRL::ComPtr<IXAudio2> xAudio2_;
	IXAudio2MasteringVoice* masterVoice_;

	std::vector<std::unique_ptr<AudioData>> audioData_;

};
