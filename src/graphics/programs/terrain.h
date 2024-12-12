//
// Created by radue on 11/15/2024.
//

#pragma once

#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>

#include "program.h"

namespace Graphics {
    class Program;
    class RenderPass;
}

namespace Memory {
    class Image;
    class Buffer;
    namespace Descriptor {
        class Set;
    }
}

class Terrain final : public Graphics::Program {
public:
    struct CreateInfo {
        const Memory::Image& heightMap;
        const Memory::Image& albedo;
        const Memory::Image& normal;
        const Memory::Buffer& particlesBuffer;
        const Memory::Buffer& lightIndicesBuffer;
    };

    explicit Terrain(const CreateInfo &createInfo);
    ~Terrain() override = default;

    void Init() override {}
    void Update(double deltaTime) override {}
    void Draw(const vk::CommandBuffer &commandBuffer, const Graphics::RenderPass *renderPass) const override;
    void ResetDescriptorSets() override;

    void OnUIAttach() override {}
    void OnUIUpdate() override {}
    void OnUIRender() override {}
    void OnUIReset() override {}
    void OnUIDetach() override {}

private:
    glm::ivec2 patchCount = { 200, 200 };

    const Memory::Image& m_heightMap;
    const Memory::Image& m_albedo;
    const Memory::Image& m_normal;

    const Memory::Buffer& m_particlesBuffer;
    const Memory::Buffer& m_lightIndicesBuffer;

    std::unique_ptr<Memory::Descriptor::Set> m_descriptorSet;
};

