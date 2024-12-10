//
// Created by radue on 11/17/2024.
//

#pragma once

#include <glm/vec2.hpp>

#include "program.h"

namespace Memory {
    class Buffer;
    class Image;
    namespace Descriptor {
        class Set;
    }
}

class Lake final : public Graphics::Program {
public:
    struct CreateInfo {
        bool depthOnly = false;
        const Memory::Buffer& particlesBuffer;
        const Memory::Buffer& lightIndicesBuffer;
    };

    explicit Lake(const CreateInfo &createInfo);
    ~Lake() override = default;

    void Init() override;
    void Update(double deltaTime) override {}
    void Draw(const vk::CommandBuffer &commandBuffer, const Graphics::RenderPass *renderPass) const override;
    void ResetDescriptorSets() override;

    void OnUIAttach() override;
    void OnUIUpdate() override {}
    void OnUIRender() override;
    void OnUIReset() override;
    void OnUIDetach() override;

private:
    bool m_depthOnly = false;
    const Memory::Buffer& m_particlesBuffer;
    const Memory::Buffer& m_lightIndicesBuffer;

    std::unique_ptr<Memory::Buffer> m_cameraBuffer;
    std::vector<std::unique_ptr<Memory::Descriptor::Set>> m_descriptorSets;
    std::unique_ptr<Memory::Image> m_spectrum;

    glm::ivec2 patchCount = { 100, 100 };

    vk::DescriptorSet spectrumDescriptorSet;
    std::vector<vk::DescriptorSet> reflectionDescriptorSets;

    std::unique_ptr<Memory::Descriptor::Set> m_descriptorSet;
};

