//
// Created by radue on 11/15/2024.
//

#pragma once

#include "components/camera.h"
#include "graphics/objects/texture.h"
#include "graphics/objects/textureArray.h"
#include "graphics/programs/program.h"
#include "memory/buffer.h"

class Terrain final : public Graphics::Program {
    struct Settings {
        float maxHeight = 5.f;
        float patchSize = .1f;
        float padding[2];
        float noiseWeights[10] = {
            1 / 512.f, 1 / 256.f, 1 / 128.f, 1 / 64.f,
            1 / 32.f, 1 / 16.f, 1 / 8.f, 1 / 4.f,
            1 / 2.f, 1.f
        };
    };
public:
    struct CreateInfo {
        const mgv::Camera &camera;
        uint32_t noiseTextureCount = 10;
    };

    explicit Terrain(Core::Device &device, Graphics::RenderPass &renderPass, const Memory::Descriptor::Pool &pool, const CreateInfo &createInfo);
    ~Terrain() override;

    void Init() override;
    void Update(double deltaTime) override;
    void Draw(const vk::CommandBuffer &commandBuffer) override;

    void InitUI() override;
    void UpdateUI() override;
    void DrawUI() override;
    void DestroyUI() override;

private:
    Core::Device &m_device;
    const mgv::Camera &m_camera;
    uint32_t m_noiseTextureCount;
    uint32_t m_selectedMipLevel = 0;

    glm::ivec2 patchCount = { 100, 100 };
    Settings m_settings;

    bool m_changed = false;
    std::unique_ptr<Memory::Buffer<Settings>> m_settingsBuffer;
    std::unique_ptr<Memory::Image> m_noiseTextures;

    std::vector<vk::DescriptorSet> m_uiTextureDescriptors;
    std::vector<vk::ImageView> m_noiseTextureViews;

    std::unique_ptr<mgv::TextureArray> m_albedoTextures;
    // std::unique_ptr<mgv::TextureArray> m_normalTextures;
};

