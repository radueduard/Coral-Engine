//
// Created by radue on 12/8/2024.
//

#pragma once
#include "program.h"

#include "graphics/objects/mesh.h"

namespace mgv {
    class Camera;
}

class DebugCamera final : public Graphics::Program {
public:
    explicit DebugCamera();
    ~DebugCamera() override = default;

    DebugCamera(const DebugCamera&) = delete;
    DebugCamera& operator=(const DebugCamera&) = delete;

    // Graphics::Program
    void Init() override;
    void Update(double deltaTime) override;
    void Draw(const vk::CommandBuffer &commandBuffer, const Graphics::RenderPass *renderPass) const override;
    void ResetDescriptorSets() override;

    // GUI::Layer
    void OnUIAttach() override {}
    void OnUIUpdate() override {}
    void OnUIRender() override {}
    void OnUIDetach() override {}
private:
    std::unordered_map<const mgv::Camera*, std::unique_ptr<mgv::Mesh>> m_meshes;
};
