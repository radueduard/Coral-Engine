//
// Created by radue on 12/11/2024.
//

#pragma once

#include "program.h"

namespace Memory {
    class Buffer;
}

class TrajectoryDisplay final : public Graphics::Program {
public:
    struct CreateInfo {
        const Memory::Buffer& particlesBuffer;
        const Memory::Buffer& trajectoriesBuffer;
    };

    explicit TrajectoryDisplay(const CreateInfo &createInfo);
    ~TrajectoryDisplay() override = default;

    // Graphics::Program
    void Init() override {}
    void Update(double deltaTime) override {}
    void Draw(const vk::CommandBuffer &commandBuffer, const Graphics::RenderPass* renderPass) const override;
    void ResetDescriptorSets() override;

    // GUI
    void OnUIAttach() override {}
    void OnUIUpdate() override {}
    void OnUIRender() override {}
    void OnUIReset() override {}
    void OnUIDetach() override {}

private:
    const Memory::Buffer& m_particlesBuffer;
    const Memory::Buffer& m_trajectoriesBuffer;

    std::unique_ptr<Memory::Descriptor::Set> m_descriptorSet;
};
