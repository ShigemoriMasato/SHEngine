#pragma once
#include <Game/Tetris/Field/Field.h>
#include <Game/Tetris/Data.h>
#include <Game/Tetris/Mino/Tetrimino.h>
#include <array>

class Player {
public:

	Player() = default;
	~Player() = default;

	void Initialize(Field* field, Tetrimino* tetrimino);
	void Update(float deltaTime, std::unordered_map<Key, bool> key);
	void SetDownTime(float dropTimer) { normalDropTime_ = dropTimer; }

	bool SpawnMino(Tetrimino::Type tetriminoType = Tetrimino::None);

	MovableMino GetMoveMino() const { return moveMino_; }
	bool GetHasMoveMino() const { return hasMoveMino_; }
	Tetrimino::Type GetHoldMino() const { return holdMino_; }

private:

	enum Rotate {
		rLeft,
		rRight
	};

	enum Direction {
		dUp,
		dRight,
		dDown,
		dLeft,
	};

private:

	void PlayerControl(float deltaTime, std::unordered_map<Key, bool> key);

	void ExecuteSRS(Rotate rotate);

	void NotAllowDown();

	void DropInit();

private:

	MovableMino moveMino_;
	Direction direction_ = Direction::dUp;
	std::pair<int, int> spawnPosition_;

	Tetrimino::Type holdMino_ = Tetrimino::Type::None;
	Vector3 holdPosition_ = Vector3(-11.0f, 6.0f, 0.0f);

	Field* field_ = nullptr;
	Tetrimino* tetrimino_ = nullptr;

	float downTimer_ = 0.0f;

	float dropTimer_ = 0.0f;
	float maxDropTime_ = 1.0f;
	float normalDropTime_ = 1.0f;				//通常の落下時間
	const float norAllowDropTime_ = 1.0f;		//notAllowDownがtrueの時の落下時間

	float dropInitTime_ = 0;
	const float maxDropInitTime_ = 20.0f;

private:

	bool hasMoveMino_ = false;
	bool notAllowDown_ = false;

	bool reqFix_ = false;

	bool gameOver_ = false;
	bool holded_ = false;
};
