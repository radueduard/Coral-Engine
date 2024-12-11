//
// Created by radue on 12/1/2024.
//

#pragma once

#include "program.h"

namespace mgv {
    class Mesh;
}

namespace Memory {
    class Image;
    class Buffer;
    namespace Descriptor {
        class Set;
    }
}

class Fireflies;
class PartitionLights;

class FirefliesDisplay final : public Graphics::Program {
public:
    struct CreateInfo {
        const Memory::Image& heightMap;
        const Memory::Buffer& particlesBuffer;
        const Memory::Buffer& lightIndicesBuffer;
        const Memory::Buffer& trajectoriesBuffer;
    };

    explicit FirefliesDisplay(const CreateInfo &createInfo);
    ~FirefliesDisplay() override = default;

    // Graphics::Program
    void Init() override;
    void Update(double deltaTime) override;
    void Draw(const vk::CommandBuffer &commandBuffer, const Graphics::RenderPass* renderPass) const override;
    void ResetDescriptorSets() override;

    // GUI
    void OnUIAttach() override {}
    void OnUIUpdate() override {}
    void OnUIRender() override {}
    void OnUIReset() override;
    void OnUIDetach() override {}

private:
    const mgv::Mesh& m_mesh;

    std::unique_ptr<Fireflies> m_firefliesProgram;
    std::unique_ptr<PartitionLights> m_partitionLightsProgram;

    const Memory::Buffer& m_particlesBuffer;
    std::unique_ptr<Memory::Descriptor::Set> m_descriptorSet;
};
