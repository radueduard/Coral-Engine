//
// Created by radue on 11/29/2024.
//

#pragma once

#include "program.h"
#include "gui/layer.h"

namespace mgv {
    class TextureArray;
}

namespace Memory {
    class Image;
    class Buffer;
    namespace Descriptor {
        class Set;
    }
}


class GenerateTerrain final : public Compute::Program, public GUI::Layer {
public:
    struct CreateInfo {
        uint32_t size = 1024;
        mgv::TextureArray& albedoTextures;
        mgv::TextureArray& normalTextures;
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

    explicit GenerateTerrain(const CreateInfo &createInfo);
    ~GenerateTerrain() override = default;

    [[nodiscard]] const Memory::Image& HeightMap() const { return *m_heightMap; }
    [[nodiscard]] const Memory::Image& Albedo() const { return *m_albedo; }
    [[nodiscard]] const Memory::Image& Normal() const { return *m_normal; }

    // Compute::Program
    void Init() override;
    void Update() override {}
    void Compute() override;
    void ResetDescriptorSets() override;

    // GUI
    void OnUIAttach() override;
    void OnUIUpdate() override;
    void OnUIRender() override;
    void OnUIReset() override;
    void OnUIDetach() override;
private:
    uint32_t m_noiseTextureCount;
    bool m_changed = true;
    Settings m_settings;

    const mgv::TextureArray& m_albedoTextures;
    const mgv::TextureArray& m_normalTextures;

    std::unique_ptr<Memory::Buffer> m_settingsBuffer;
    std::unique_ptr<Memory::Image> m_noiseTextures;

    std::unique_ptr<Memory::Image> m_heightMap;
    std::unique_ptr<Memory::Image> m_albedo;
    std::unique_ptr<Memory::Image> m_normal;

    std::unique_ptr<Memory::Descriptor::Set> m_descriptorSet;

    // GUI
    uint32_t m_selectedMipLevel = 0;
    std::vector<vk::DescriptorSet> m_uiTextureDescriptors;
    std::vector<vk::ImageView> m_noiseTextureViews;

    vk::DescriptorSet m_heightMapDescriptorSet;
    vk::DescriptorSet m_albedoDescriptorSet;
    vk::DescriptorSet m_normalDescriptorSet;
};
