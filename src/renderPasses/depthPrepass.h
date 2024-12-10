//
// Created by radue on 12/6/2024.
//

#pragma once

#include <memory>

#include <vulkan/vulkan.hpp>

namespace Graphics {
    class RenderPass;
}

class DepthPrepass {
public:
    explicit DepthPrepass();
    ~DepthPrepass() = default;

    [[nodiscard]] Graphics::RenderPass *RenderPass() const { return m_renderPass.get(); }
    void Run(const vk::CommandBuffer &commandBuffer) const;

private:
    std::unique_ptr<Graphics::RenderPass> m_renderPass;
    uint32_t m_imageCount;
    vk::Extent2D m_extent;
    bool m_resized = false;
};
