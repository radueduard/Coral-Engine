//
// Created by radue on 11/29/2024.
//

#pragma once

#include "graphics/renderPass.h"

class ReflectionPass final {
public:
    explicit ReflectionPass();
    ~ReflectionPass() = default;

    [[nodiscard]] Graphics::RenderPass &RenderPass() const { return *m_renderPass; }
    void Run(const vk::CommandBuffer &commandBuffer) const;

private:
    std::unique_ptr<Graphics::RenderPass> m_renderPass;
    uint32_t m_imageCount;
    vk::Extent2D m_extent;
    bool m_resized = false;

    std::vector<vk::DescriptorSet> m_descriptorSets;
};
