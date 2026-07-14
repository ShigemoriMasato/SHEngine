#pragma once
#include <SHEngine.h>
#include <Render/Renderer.h>
#include <Camera/Camera.h>
#include "Animation.h"
#include "HitEffect.h"
#include "Cylinder.h"
#include "GPUParticle.h"

class Subject {
public:

	void Initialize(SHEngine::Engine* engine);
	void Update(Camera* camera);
	void Draw(DCC* cmdObj);

private:

	SHEngine::Engine* engine_;

	std::unique_ptr<SHEngine::BufferContainer> container_;

	std::unique_ptr<SHEngine::Renderer> cube_;
	SHEngine::GPUBuffer* wvp_;

	std::unique_ptr<Animation_Sub> animation_;
	std::unique_ptr<HitEffect> hitEffect_;
	std::unique_ptr<Cylinder> cylinder_;

	std::unique_ptr<GPUParticle> gpuParticle_;
};
