#pragma once
#include <Camera/Camera.h>
#include <Game/Effect/Effect.h>

class StageData {
public:

	void Load(const std::string& stageName);

private:

	struct EmitterData {
		EmitterType type;
		std::string modelPath;

		int emitNum = 0;
		Transform transform;
		Vector4 color = { 1, 1, 1, 1 };
	};

	static inline const std::string basePath_ = "Game/StageConfig/";

	std::string currentStageName_;

	CameraCurveData introCamera_;
	CameraCurveData playingCamera_;

	std::vector<EmitterData> emitterDataList_;
};
