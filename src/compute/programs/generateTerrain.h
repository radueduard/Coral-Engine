//
// Created by radue on 11/29/2024.
//

#pragma once
#include "program.h"
#include "compute/pipeline.h"
#include "graphics/objects/texture.h"
#include "memory/buffer.h"
#include "memory/image.h"

class GenerateTerrain final : public Compute::Program {
public:
    struct CreateInfo {
        uint32_t size = 1024;
    };

    struct Settings {
        float maxHeight = 5.f;
        uint32_t lodCount = 10;
        float padding[2] {};
        float noiseWeights[20] = { 1.f };
    };

    struct Limits {
        float maxHeight;
        float padding[3];
        float minHeight;
    };

    GenerateTerrain(const Memory::Descriptor::Pool &pool, const CreateInfo &createInfo);
    ~GenerateTerrain() override = default;

    // Compute::Program
    void Init() override;
    void Update() override;
    void Compute(const vk::CommandBuffer &commandBuffer) override;

    [[nodiscard]] const Memory::Image* HeightMap() const { return m_heightMap.get(); }

    // GUI
    void InitUI() override;
    void UpdateUI() override;
    void DrawUI() override;
    void DestroyUI() override;
private:
    uint32_t m_noiseTextureCount;

    bool m_changed = true;
    Settings m_settings;
    std::unique_ptr<Memory::Buffer<Settings>> m_settingsBuffer;
    std::unique_ptr<Memory::Image> m_noiseTextures;

    std::unique_ptr<Memory::Image> m_heightMap;

    std::unique_ptr<Memory::Descriptor::Set> m_descriptorSet;

    // GUI
    uint32_t m_selectedMipLevel = 0;
    std::vector<vk::DescriptorSet> m_uiTextureDescriptors;
    std::vector<vk::ImageView> m_noiseTextureViews;

    vk::DescriptorSet m_heightMapDescriptorSet;
};
