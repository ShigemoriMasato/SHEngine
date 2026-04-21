#pragma once
#include "ScoreManager.h"

class LevelManager {
public:

	LevelManager(ScoreManager* scoreManager);

	void Initialize(int startLevel);
	void FallUpdate();

private:

	ScoreManager* scoreManager = nullptr;
	int level = 0;
	void LevelUp();
	void UpdateFallInterval();

};

