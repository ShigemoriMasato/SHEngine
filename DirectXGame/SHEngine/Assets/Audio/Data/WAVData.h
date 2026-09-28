#pragma once
#include "AudioData.h"

/**
 * @class WAVData
 * @brief WAV形式の音声データを扱うクラス
 * 
 * AudioDataクラスを継承し、WAVファイルの読み込みと再生機能を実装する。
 * RIFFフォーマットに従ってWAVファイルを解析し、XAudio2で再生可能な形式に変換する。
 */
class WAVData : public AudioData {
public:

	WAVData(IXAudio2* xAudio, std::string filePath);
	~WAVData() = default;

private:

	// チャンクヘッダ
	struct ChunkHeader {
		char id[4];   // チャンク毎のID
		int32_t size; // チャンクサイズ
	};

	// RIFFヘッダチャンク
	struct RiffHeader {
		ChunkHeader chunk; // RIFF
		char type[4];  // WAVE
	};

	// FMTチャンク
	struct FormatChunk {
		ChunkHeader chunk; // fmt
		WAVEFORMATEX fmt;  // 波形フォーマット
	};

};

