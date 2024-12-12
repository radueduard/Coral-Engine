//
// Created by radue on 12/11/2024.
//

#pragma once

#include "program.h"

namespace Memory {
    class Buffer;
    class Image;
}

class TrajectoryDisplay final : public Graphics::Program {
public:
    struct CreateInfo {
        const Memory::Buffer& particlesBuffer;
        const Memory::Buffer& trajectoriesBuffer;
        const Memory::Image& heightMap;
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
    void OnUIRender() override;
    void OnUIReset() override {}
    void OnUIDetach() override {}

private:
    bool m_enabled = false;

    const Memory::Buffer& m_particlesBuffer;
    const Memory::Buffer& m_trajectoriesBuffer;
    const Memory::Image& m_heightMap;

    std::unique_ptr<Memory::Descriptor::Set> m_descriptorSet;
};
