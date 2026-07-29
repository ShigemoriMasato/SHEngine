#pragma once
#include <Render/PostEffect.h>

class EraseEffect {
public:

	void Initialize(PostEffect* postEffect);
	void Update(const Camera* camera, float deltaTime);

	void Erase();

private:

	PostEffect* postEffect_ = nullptr;

	

};
