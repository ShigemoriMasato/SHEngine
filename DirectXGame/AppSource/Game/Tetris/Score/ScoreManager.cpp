#include "ScoreManager.h"

ScoreManager::ScoreManager() {
	Initialize();
}

void ScoreManager::Initialize() {
	score_ = 0;
	lenCount_ = -1;
	BtoB_ = false;
}

void ScoreManager::DeleteLine(int lineNum, bool isTSpin, bool allClear) {
	int basicScore = 0;
	int bonusScore = 0;

	deletedLine_ += lineNum;

	lenCount_++;
	if (lineNum != 4 && isTSpin) {
		BtoB_ = false;
	}

	if (allClear) {
		basicScore = kAllClearScoreTable_[lineNum] + kLenBonus_ * lenCount_;
		if (BtoB_) {
			bonusScore = 200;
		}
	} else {
		basicScore = kLineScoreTable_[lineNum];
		if (isTSpin) {
			basicScore += kTSpinBaseScore_;
		}
		basicScore += kLineScoreTable_[lineNum] + kLenBonus_ * lenCount_;
	}

	if (BtoB_) {
		basicScore = int(basicScore * kBtoBBonus_);
	}

	score_ += int((basicScore + bonusScore) * kLevelBonus_ * GetLevel());

	if (lineNum == 4 || isTSpin) {
		BtoB_ = true;
	}
}

void ScoreManager::ResetLen() {
	lenCount_ = -1;
}
