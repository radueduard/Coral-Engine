//
// Created by radue on 12/2/2024.
//

#pragma once

#include <memory>
#include <vector>

#include <glm/glm.hpp>

#include "program.h"
#include "gui/layer.h"

namespace Memory {
    class Buffer;
    class Image;
    namespace Descriptor {
        class Set;
    }
}

namespace Graphics {
    class RenderPass;
}

struct Indices {
    glm::uint indices[64];
};

class PartitionLights final : public Compute::Program, public GUI::Layer {
public:
    struct CreateInfo {
        glm::uvec2 chunksPerAxis;
        const Memory::Buffer& particlesBuffer;
        const Memory::Buffer& lightIndicesBuffer;
        const Memory::Buffer& frustumsBuffer;
    };

    explicit PartitionLights(const CreateInfo &createInfo);
    ~PartitionLights() override = default;

    // Compute::Program
    void Init() override {}
    void Update() override {}
    void Compute() override;
    void ResetDescriptorSets() override;

    // GUI::Layer
    void OnUIAttach() override;
    void OnUIUpdate() override {}
    void OnUIRender() override;
    void OnUIReset() override;
    void OnUIDetach() override;

private:
    glm::uvec2 m_chunksPerAxis;
    std::vector<std::unique_ptr<Memory::Descriptor::Set>> m_descriptorSets;

    const Memory::Buffer& m_particlesBuffer;
    const Memory::Buffer& m_lightIndicesBuffer;
    const Memory::Buffer& m_frustumsBuffer;

    std::unique_ptr<Memory::Image> m_debugImage;
    vk::DescriptorSet m_debugDescriptorSet;
};
