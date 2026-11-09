#pragma once
#include <Render/Buffer/GPUBuffer.h>
#include <Render/Renderer.h>
#include <Tool/Binary/binaryManager.h>
#include <SHEngine.h>

class IPostEffect {
public:
    virtual ~IPostEffect() = default;

    virtual void Initialize(SHEngine::Engine& engine) = 0;
    virtual void DrawImGui() = 0;
    virtual void Save(BinaryManager& json) const = 0;
    virtual void Load(BinaryManager& json) = 0;

    virtual void Apply(SHEngine::Screen::IDisplay& source, SHEngine::Screen::IDisplay& destination) = 0;

    bool IsEnabled() const { return enabled_; }
    void SetEnabled(bool enabled) { enabled_ = enabled; }

private:
    bool enabled_ = true;
};