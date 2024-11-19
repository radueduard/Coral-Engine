//
// Created by radue on 11/5/2024.
//

#pragma once

#include "graphics/renderPass.h"
#include "gui/layer.h"

class MainViewport final : public GUI::Layer {
public:
    explicit MainViewport(Core::Device &device);
    ~MainViewport() override = default;

    [[nodiscard]] Graphics::RenderPass &RenderPass() const { return *m_renderPass; }
    void Run(const vk::CommandBuffer &commandBuffer) const;

    void InitUI() override;
    void UpdateUI() override;
    void DrawUI() override;
    void DestroyUI() override;

private:
    Core::Device &m_device;
    std::unique_ptr<Graphics::RenderPass> m_renderPass;
    uint32_t m_imageCount;
    vk::Extent2D m_extent;
    bool m_resized = false;

    std::vector<vk::DescriptorSet> m_descriptorSets;
};
