//
// Created by radue on 12/2/2024.
//

#pragma once
#include "fireflies.h"

struct Indices {
    glm::uint indices[16];
};

class PartitionLights : public Compute::Program {
public:
    struct CreateInfo {
        uint32_t chunksPerAxis;
        const Memory::Buffer<Fireflies::Particle>& particlesBuffer;
    };

    PartitionLights(const Memory::Descriptor::Pool &pool, const CreateInfo &createInfo);
    ~PartitionLights() override = default;

    // Compute::Program
    void Init() override;
    void Update() override;
    void Compute(const vk::CommandBuffer &commandBuffer) override;

    // GUI
    void InitUI() override {}
    void UpdateUI() override {}
    void DrawUI() override {}
    void DestroyUI() override {}
private:
    uint32_t m_chunksPerAxis;
    std::unique_ptr<Memory::Descriptor::Set> m_descriptorSet;
    std::unique_ptr<Memory::Buffer<Indices>> m_lightIndicesBuffer;
};
