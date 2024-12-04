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
    struct CreateInfo {
        const mgv::Camera &camera;
        const Memory::Image* heightMap;
        const Memory::Buffer<Fireflies::Particle>& particlesBuffer;
        const Memory::Buffer<Indices>& lightIndicesBuffer;
    };

    FirefliesDisplay(Graphics::RenderPass& renderPass, const Memory::Descriptor::Pool &pool, const FirefliesDisplay::CreateInfo &createInfo);
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

private:
    const mgv::Camera& m_camera;
    const Memory::Buffer<Fireflies::Particle>& m_particlesBuffer;
    const mgv::Mesh& m_mesh;
    std::unique_ptr<Fireflies> m_firefliesProgram;
    std::unique_ptr<PartitionLights> m_partitionLightsProgram;
};
