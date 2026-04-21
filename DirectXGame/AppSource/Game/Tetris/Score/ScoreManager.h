#pragma once
class ScoreManager {
public:

	ScoreManager() = default;
	~ScoreManager() = default;
	void Initialize();

	void DeleteLine(int lineNum, bool isTSpin);

private:

	int score = 0;

};
