#pragma once
#include <Render/RenderObject.h>
#include <Render/Screen/MultiDisplay.h>
#include "PostEffectData.h"

enum class PostEffectJob : uint32_t {
	None = 0,
	BlurV = 1 << 0,
	BlurH = 1 << 1,
	Fade = 1 << 2,
	Glitch = 1 << 3,
	GrayScale = 1 << 4,
	GridTransition = 1 << 5,
	SlowMotion = 1 << 6,
	HeavyBlur = 1 << 7,
};

uint32_t operator|(PostEffectJob a, PostEffectJob b);
uint32_t operator|(uint32_t a, PostEffectJob b);
uint32_t operator&(uint32_t a, PostEffectJob b);
uint32_t operator~(PostEffectJob a);
bool operator<(PostEffectJob a, PostEffectJob b);

struct PostEffectConfig {
	CmdObj* cmdObj = nullptr;
	SHEngine::Screen::IDisplay* origin = nullptr;
	SHEngine::Screen::IDisplay* output = nullptr;	//nullptrの場合はoriginに描画する

	uint32_t jobs_ = 0;

};

class PostEffect {
public:

	void Initialize(SHEngine::TextureManager* textureManager, SHEngine::DrawData drawData);
	template<typename T>
	void CopyBuffer(PostEffectJob job, const T& data);
	void Draw(const PostEffectConfig& config);

private:

	std::unique_ptr<SHEngine::Screen::MultiDisplay> intermediateDisplay_ = nullptr;
	std::map<PostEffectJob, std::unique_ptr<SHEngine::RenderObject>> postEffectObjects_{};

};

template<typename T>
inline void PostEffect::CopyBuffer(PostEffectJob job, const T& data) {
	auto& postEffectObject = postEffectObjects_.at(job);
	postEffectObject->CopyBufferData(1, &data, sizeof(T));
}
