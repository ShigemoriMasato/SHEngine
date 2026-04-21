#pragma once
#include <vector>
#include <array>
#include <Camera/Camera.h>

class Field {
public:

	Field(int width, int height);
	~Field() = default;

	void Initialize(Camera* camera);

	std::vector<std::vector<int>> GetField() const;

	bool SetFieldIndex(int x, int y, int type);
	void LineCheck();
	void DeleteFillLine();

	std::vector<int> FillLineIndex() const { return reqLine_; }

private:

	std::vector<std::vector<int>> field_;
	std::array<std::pair<int, int>, 4> currentMino_{};
	const int width_;
	const int height_;

	std::vector<int> reqLine_{};

};
