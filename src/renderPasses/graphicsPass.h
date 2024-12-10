//
// Created by radue on 11/5/2024.
//

#pragma once

#include <memory>
#include <vulkan/vulkan.hpp>

namespace Graphics {
    class RenderPass;
}

class GraphicsPass final {
public:
    explicit GraphicsPass();
    ~GraphicsPass() = default;

    [[nodiscard]] Graphics::RenderPass *RenderPass() const { return m_renderPass.get(); }
    void Run(const vk::CommandBuffer &commandBuffer) const;

private:
    std::unique_ptr<Graphics::RenderPass> m_renderPass;
    uint32_t m_imageCount;
    vk::Extent2D m_extent;
    bool m_resized = false;
};
