#include "WAVData.h"
#include <cassert>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

WAVData::WAVData(IXAudio2* xAudio, std::string filePath) : AudioData(xAudio) {
	// ファイル入力ストリームのインスタンス
	std::ifstream file;
	// .wavファイルをバイナリモードで開く
	file.open(filePath, std::ios_base::binary);
	// ファイルオープン失敗を検出する
	assert(file.is_open());

	// RIFFヘッダーの読み込み
	RiffHeader riff;
	file.read((char*)&riff, sizeof(riff));
	// ファイルがRIFFかチェック
	if (strncmp(riff.chunk.id, "RIFF", 4) != 0) {
		assert(0);
	}
	// タイプがWAVRかチェック
	if (strncmp(riff.type, "WAVE", 4) != 0) {
		assert(0);
	}

	FormatChunk format = {};
	ChunkHeader chunk = {};

	// チャンクを順に読みながらfmtチャンクを探す
	while (file.read((char*)&chunk, sizeof(chunk))) {
		if (strncmp(chunk.id, "fmt ", 4) == 0) {
			// サイズが適正か確認
			assert(chunk.size <= sizeof(format.fmt));
			format.chunk = chunk;
			file.read((char*)&format.fmt, chunk.size);
			break;
		} else {
			// 必要のないチャンクは読み飛ばす
			file.seekg(chunk.size, std::ios_base::cur);
		}
	}

	// fmtチャンクが見つからなければエラー
	if (strncmp(format.chunk.id, "fmt ", 4) != 0) {
		assert(0);
	}

	// Dataチャンクの読み込み
	ChunkHeader data;
	file.read((char*)&data, sizeof(data));
	// JUNKチャンクを検出した場合
	if (strncmp(data.id, "JUNK", 4) == 0) {
		// 読み取り位置をJUNKチャンクの終わりまで進める
		file.seekg(data.size, std::ios_base::cur);
		// 再読み込み
		file.read((char*)&data, sizeof(data));

	}

	if (strncmp(data.id, "data", 4) != 0) {
		assert(0);
	}
	// Dataチャンクのデータ部（波形データ）の読み込み
	auto pBuffer = std::make_unique<BYTE[]>(data.size);
	file.read(reinterpret_cast<char*>(pBuffer.get()), data.size);
	// Waveファイルを閉じる
	file.close();

	// returnする為の音声データ
	wfex_ = format.fmt;
	audioData_.resize(data.size);
	std::memcpy(audioData_.data(), pBuffer.get(), data.size);
	name_ = filePath;
}