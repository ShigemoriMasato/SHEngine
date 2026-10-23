#pragma once
#include <array>

class ScoreManager {
public:

	ScoreManager();
	~ScoreManager() = default;

	void Initialize();

	void DeleteLine(int lineNum, bool isTSpin, bool allClear);
	void ResetLen();

	int GetScore() const { return score_; }
	int GetLen() const { return lenCount_; }
	int GetLevel() const { return deletedLine_ / kLevelPartition + 1; }

private:

	int score_ = 0;
	int lenCount_ = -1;		//連続消しの回数
	bool BtoB_ = false;
	const float kLevelBonus_ = 1.0f;

	int deletedLine_ = 40;
	const int kLevelPartition = 5;

	const std::array<int, 5> kLineScoreTable_ = { 0, 100, 300, 500, 800 };

	const float kBtoBBonus_ = 1.5f;

	const int kLenBonus_ = 50;

	const float kTSpinBonus_ = 2.0f;
	const int kTSpinBaseScore_ = 600;

	const std::array<int, 6> kAllClearScoreTable_ = { 0, 900, 1500, 2300, 2800 };
};
