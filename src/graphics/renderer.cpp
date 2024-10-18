//
// Created by radue on 10/14/2024.
//

#include "renderer.h"

#include <iostream>

namespace Graphics {
    Renderer::Renderer(const Core::Window &window, const Core::Device &device)
        : m_window(window), m_device(device) {
        m_swapChain = std::make_unique<Graphics::SwapChain>(
            device,
            window.Extent());
    }

    bool Renderer::BeginFrame() {
        if (m_frameStarted) {
            std::cerr << "Called BeginFrame() while frame is already started!" << std::endl;
        }

        if (const auto result = m_swapChain->Acquire();
            result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR) {
            auto newSwapChain = std::make_unique<Graphics::SwapChain>(
                m_device,
                m_window.Extent(),
                std::move(m_swapChain));
            m_swapChain = std::move(newSwapChain);
            return false;
        }

        m_frameStarted = true;
        return true;
    }

    void Renderer::Draw(std::function<void(vk::CommandBuffer)> drawFunc) const {
        if (!m_frameStarted) {
            std::cerr << "Called Draw() while frame is not started!" << std::endl;
        }

        const auto &frame = m_swapChain->CurrentFrame();
        const auto &commandBuffer = frame.commandBuffers.at(vk::QueueFlagBits::eGraphics);
        const auto &renderPass = m_swapChain->RenderPass();

        const auto clearValues = {
            vk::ClearValue().setColor(vk::ClearColorValue().setFloat32({0.0f, 0.0f, 0.0f, 1.0f}))
        };

        const auto renderArea = vk::Rect2D()
            .setOffset(vk::Offset2D())
            .setExtent(m_swapChain->Extent());

        const auto beginInfo = vk::RenderPassBeginInfo()
            .setRenderPass(renderPass)
            .setFramebuffer(frame.framebuffer)
            .setRenderArea(renderArea)
            .setClearValues(clearValues);

        const auto viewport = vk::Viewport()
            .setX(0.0f)
            .setY(0.0f)
            .setWidth(static_cast<float>(m_window.Extent().width))
            .setHeight(static_cast<float>(m_window.Extent().height))
            .setMinDepth(0.0f)
            .setMaxDepth(1.0f);

        const auto scissor = vk::Rect2D()
            .setOffset({0, 0})
            .setExtent(m_window.Extent());

        commandBuffer.beginRenderPass(beginInfo, vk::SubpassContents::eInline);
        commandBuffer.setViewport(0, viewport);
        commandBuffer.setScissor(0, scissor);

        drawFunc(commandBuffer);

        commandBuffer.endRenderPass();
    }

    bool Renderer::EndFrame() {
        if (!m_frameStarted) {
            std::cerr << "Called EndFrame() while frame is not started!" << std::endl;
        }
        m_swapChain->Submit();
        m_frameStarted = false;

        if (const auto result = m_swapChain->Present();
            result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR) {
            auto newSwapChain = std::make_unique<Graphics::SwapChain>(
                m_device,
                m_window.Extent(),
                std::move(m_swapChain));
            m_swapChain = std::move(newSwapChain);
            return false;
        }

        return true;
    }
}