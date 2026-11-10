#pragma once
#include <Render/Buffer/GPUBuffer.h>
#include <Render/Renderer.h>
#include <Tool/Binary/binaryManager.h>
#include "PostEffectContext.h"

class IPostEffect {
public:
    virtual ~IPostEffect() = default;

    virtual void Initialize(SHEngine::Engine& engine) = 0;
    virtual void DrawImGui() = 0;
    virtual void Save(BinaryManager& bin) const {};
    virtual void Load(BinaryManager& bin) {};

    virtual void Apply(PostEffectContext* context) = 0;

    bool IsEnabled() const { return enabled_; }
    void SetEnabled(bool enabled) { enabled_ = enabled; }

private:
    bool enabled_ = true;
};