//
// Created by radue on 11/17/2024.
//

#pragma once


#include "components/camera.h"
#include "compute/programs/fireflies.h"
#include "compute/programs/partitionLights.h"
#include "graphics/objects/texture.h"
#include "graphics/objects/textureArray.h"
#include "graphics/programs/program.h"
#include "memory/buffer.h"

struct CameraData {
    glm::mat4 view;
    glm::mat4 projection;
    glm::mat4 inverseView;
    glm::mat4 inverseProjection;
    glm::mat4 flippedView;
    glm::mat4 flippedInverseView;
};

class Lake final : public Graphics::Program {
public:
    struct CreateInfo {
        const mgv::Camera &camera;
        const Graphics::RenderPass &reflectionPass;
        const Memory::Buffer<Fireflies::Particle>& particlesBuffer;
        const Memory::Buffer<Indices>& lightIndicesBuffer;
    };

    explicit Lake(Graphics::RenderPass &renderPass, const Memory::Descriptor::Pool &pool, const CreateInfo &createInfo);
    ~Lake() override = default;

    void Init() override;
    void Update(double deltaTime) override;
    void Draw(const vk::CommandBuffer &commandBuffer, bool reflected) override;

    void InitUI() override;
    void UpdateUI() override;
    void DrawUI() override;
    void DestroyUI() override;

private:
    const mgv::Camera &m_camera;
    std::vector<const Memory::Image*> m_reflectionTextures;

    std::unique_ptr<Memory::Buffer<CameraData>> m_cameraBuffer;

    std::vector<std::unique_ptr<Memory::Descriptor::Set>> m_descriptorSets;

    std::unique_ptr<Memory::Image> m_spectrum;

    glm::ivec2 patchCount = { 100, 100 };

    vk::DescriptorSet spectrumDescriptorSet;
    std::vector<vk::DescriptorSet> reflectionDescriptorSets;
};

