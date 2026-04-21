#pragma once
#include <Common/KeyConfig/KeyManager.h>

//KeyConfigからもらったっ情報をもとにさらに細かい設定を行うとこ
class KeyCoating {
public:

	KeyCoating(KeyManager* manager) : keyManager_(manager) {}

	void Update(float deltaTime);

	std::unordered_map<Key, bool> GetKeyStates() const {
		return keyFlugs_;
	}

private:

	KeyManager* keyManager_{ nullptr };
	std::unordered_map<Key, bool> keyFlugs_{};

private:

	float timer[4] = {};
	bool preFlugs_[4] = {};
	const float initialTime = 0.4f;
	const float intervalTime = 0.07f;

};
