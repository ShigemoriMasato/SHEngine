#pragma once
#include <utility>
#include <Common/KeyConfig/KeyManager.h>
#include <random>

//テトリミノに関する管理。7種1順をここで管理する
class Tetrimino {
public:

	enum Type {
		None,
		T,
		S,
		Z,
		L,
		J,
		I,
		O,
		Wall,
		Del,
		Hold,
		Next,
		Count
	};

	Tetrimino() = default;
	~Tetrimino() = default;

	void Initialize(std::mt19937 g);

	std::vector<std::pair<int, int>> GetOffset(Type type);

	/// <summary>
	/// ネクストが1進んだ場合に呼ぶ
	/// </summary>
	/// <returns>削除されたネクスト</returns>
	Type PopFirst();

	/// <summary>
	/// 次のテトリミノを取得する
	/// </summary>
	/// <param name="nextNum">16まで</param>
	Type GetNextTetrimino(int nextNum);

	std::vector<std::pair<int, int>> GetSRSOffset(int preDir, bool isRight);

private:

	//Nextの抽選
	void RefillNextBuffer();

private:

	std::map<Type, std::vector<std::pair<int, int>>> blockOffsets_ = {
		{ Type::T, { {1, 0}, {0, 0}, {0, 1}, {-1, 0} } },
		{ Type::S, { {1, 0}, {0, 0}, {0, 1}, {-1, 1} } },
		{ Type::Z, { {1, 1}, {0, 1}, {0, 0}, {-1, 0} } },
		{ Type::L, { {1, 0}, {0, 0}, {-1, 0}, {-1, 1} } },
		{ Type::J, { {1, 1}, {1, 0}, {0, 0}, {-1, 0} } },
		{ Type::I, { {2, 0}, {1, 0}, {0, 0}, {-1, 0} } },
		{ Type::O, { {0, 0}, {1, 0}, {0, 1}, {1, 1} } }
	};

	std::vector<Type> nexts_;
	std::vector<Type> nextBuffer_;

	std::mt19937 radg_;

	const int kMinoTypeNum_ = 7;

	std::array<std::array<std::vector<std::pair<int, int>>, 2>, 4> srsOffsets_ = {};

};
