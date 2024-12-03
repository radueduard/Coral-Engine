//
// Created by radue on 12/1/2024.
//

#pragma once

#include "program.h"
#include "components/camera.h"

#include "compute/programs/fireflies.h"
#include "compute/programs/partitionLights.h"

class FirefliesDisplay final : public Graphics::Program {
public:
    FirefliesDisplay(Graphics::RenderPass& renderPass, const Memory::Descriptor::Pool &pool, const Fireflies::CreateInfo &createInfo);
    ~FirefliesDisplay() override = default;

    // Graphics::Program
    void Init() override;
    void Update(double deltaTime) override {}
    void Draw(const vk::CommandBuffer &commandBuffer, bool reflected) override;

    // GUI
    void InitUI() override {}
    void UpdateUI() override {}
    void DrawUI() override {}
    void DestroyUI() override {}

    [[nodiscard]] const Memory::Buffer<Fireflies::Particle>& ParticlesBuffer() const { return m_firefliesProgram->ParticlesBuffer(); }
    [[nodiscard]] const Memory::Buffer<Indices>& LightIndicesBuffer() const { return m_partitionLightsProgram->LightIndicesBuffer(); }
    [[nodiscard]] uint32_t Count() const { return m_firefliesProgram->Count(); }

private:
    const mgv::Camera& m_camera;
    const mgv::Mesh* m_mesh = nullptr;
    std::unique_ptr<Fireflies> m_firefliesProgram;
    std::unique_ptr<PartitionLights> m_partitionLightsProgram;
};
