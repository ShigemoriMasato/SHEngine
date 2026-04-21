#include "MP3Data.h"
#include <cassert>
#include <Utility/ConvertString.h>

#include <mfapi.h>
#include <mfidl.h>
#include <filesystem>
#include <mfreadwrite.h>

namespace fs = std::filesystem;

MP3Data::MP3Data(IXAudio2* xAudio, std::string filePath) : AudioData(xAudio) {
	// 音声データを登録
	name_ = filePath;

	std::wstring path = ConvertString(name_);

	// ソースリーダーの作成
	IMFSourceReader* pMFSourceReader{ nullptr };
	HRESULT hr = MFCreateSourceReaderFromURL(path.c_str(), NULL, &pMFSourceReader);
	assert(SUCCEEDED(hr));

	// メディアタイプを取得
	IMFMediaType* pMFMediaType{ nullptr };
	hr = MFCreateMediaType(&pMFMediaType);
	assert(SUCCEEDED(hr));

	pMFMediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
	pMFMediaType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
	pMFSourceReader->SetCurrentMediaType(static_cast<DWORD>(MF_SOURCE_READER_FIRST_AUDIO_STREAM), nullptr, pMFMediaType);

	pMFMediaType->Release();
	pMFMediaType = nullptr;

	pMFSourceReader->GetCurrentMediaType(static_cast<DWORD>(MF_SOURCE_READER_FIRST_AUDIO_STREAM), &pMFMediaType);

	WAVEFORMATEX* waveFormat{ nullptr };
	hr = MFCreateWaveFormatExFromMFMediaType(pMFMediaType, &waveFormat, nullptr);
	assert(SUCCEEDED(hr));

	// データ読み込み
	std::vector<BYTE>& bufferData = audioData_;
	while (true) {
		IMFSample* pMFSample{ nullptr };
		DWORD dwStreamFlags = 0;
		pMFSourceReader->ReadSample(static_cast<DWORD>(MF_SOURCE_READER_FIRST_AUDIO_STREAM), 0, nullptr, &dwStreamFlags, nullptr, &pMFSample);

		if (dwStreamFlags & MF_SOURCE_READERF_ENDOFSTREAM) {
			break;
		}

		IMFMediaBuffer* pMFMediaBuffer{ nullptr };
		pMFSample->ConvertToContiguousBuffer(&pMFMediaBuffer);

		BYTE* pBuffer = nullptr;
		DWORD cbCurrentLength = 0;
		pMFMediaBuffer->Lock(&pBuffer, nullptr, &cbCurrentLength);
		// データを一時バッファに追加
		bufferData.insert(bufferData.end(), pBuffer, pBuffer + cbCurrentLength);

		// 解放処理
		pMFMediaBuffer->Unlock();
		pMFMediaBuffer->Release();
		pMFSample->Release();

	}

	// 読み込んだデータをメモリにコピー
	wfex_ = *waveFormat;

	// 解放処理
	CoTaskMemFree(waveFormat);
	pMFMediaType->Release();
	pMFSourceReader->Release();
}
