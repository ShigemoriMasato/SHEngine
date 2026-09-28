#pragma once
#include "AudioData.h"

class MP3Data : public AudioData {
public:

	MP3Data(IXAudio2* xAudio, std::string filePath);
	~MP3Data() = default;

private:

};

