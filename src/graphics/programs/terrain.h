//
// Created by radue on 11/15/2024.
//

#pragma once

#include "components/camera.h"
#include "compute/programs/generateTerrain.h"
#include "graphics/objects/textureArray.h"
#include "graphics/programs/program.h"

#include "compute/programs/fireflies.h"
#include "compute/programs/partitionLights.h"

class Terrain final : public Graphics::Program {
public:
    struct CreateInfo {
        const mgv::Camera &camera;
        const Memory::Image& heightMap;
        const Memory::Image& albedo;
        const Memory::Image& normal;
        const Memory::Buffer<Fireflies::Particle>& particlesBuffer;
        const Memory::Buffer<Indices>& lightIndicesBuffer;
    };

    explicit Terrain(Graphics::RenderPass &renderPass, const Memory::Descriptor::Pool &pool, const CreateInfo &createInfo);
    ~Terrain() override;

    void Init() override;
    void Update(double deltaTime) override;
    void Draw(const vk::CommandBuffer &commandBuffer, bool reflected) override;

    void InitUI() override;
    void UpdateUI() override;
    void DrawUI() override;
    void DestroyUI() override;

private:
    const mgv::Camera &m_camera;

    glm::ivec2 patchCount = { 100, 100 };
};

