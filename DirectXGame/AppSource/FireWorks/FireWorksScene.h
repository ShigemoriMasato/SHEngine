#pragma once
#include <Scene/IScene.h>

class FireWorksScene : public IScene {
public:

	~FireWorksScene() override = default;

	void Initialize() override;
	std::unique_ptr<IScene> Update() override;
	void Draw() override;

private:

	

};
