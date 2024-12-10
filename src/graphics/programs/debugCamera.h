//
// Created by radue on 12/8/2024.
//

#pragma once
#include "program.h"

namespace mgv {
    class Camera;
}

class DebugCamera final : public Graphics::Program {
public:
    explicit DebugCamera(const mgv::Camera& camera);
    ~DebugCamera() override = default;

    DebugCamera(const DebugCamera&) = delete;
    DebugCamera& operator=(const DebugCamera&) = delete;

    // Graphics::Program
    void Init() override;
    void Update(double deltaTime) override;
    void Draw(const vk::CommandBuffer &commandBuffer, const Graphics::RenderPass *renderPass) const override;
    void ResetDescriptorSets() override;
private:
    const mgv::Camera& m_camera;
};
