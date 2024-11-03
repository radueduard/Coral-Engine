//
// Created by radue on 10/14/2024.
//

#include "renderer.h"

#include <iostream>

#include "swapChain.h"

namespace Graphics {
    Renderer::Renderer(const Core::Window &window, const Core::Device &device)
        : m_window(window), m_device(device) {
        m_swapChain = std::make_unique<Graphics::SwapChain>(
            device,
            window.Extent());
        CreateDescriptorPool();
    }

    void Renderer::CreateDescriptorPool() {
        m_descriptorPool = Memory::Descriptor::Pool::Builder(m_device)
            .AddPoolSize(vk::DescriptorType::eUniformBuffer, 100)
            .AddPoolSize(vk::DescriptorType::eStorageBuffer, 100)
            .AddPoolSize(vk::DescriptorType::eCombinedImageSampler, 100)
            .PoolFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet)
            .MaxSets(100)
            .Build();
    }

    bool Renderer::BeginFrame() {
        if (m_frameStarted) {
            std::cerr << "Called BeginFrame() while frame is already started!" << std::endl;
        }

        if (m_window.IsPaused()) {
            return false;
        }

        if (const auto result = m_swapChain->Acquire();
            result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR) {
            auto newSwapChain = std::make_unique<Graphics::SwapChain>(
                m_device,
                m_window.Extent(),
                std::move(m_swapChain));
            m_swapChain = std::move(newSwapChain);
            m_frameStarted = false;
        } else {
            m_frameStarted = true;
        }
        return m_frameStarted;
    }

    void Renderer::Draw(const std::function<void(vk::CommandBuffer)> &drawFunc) const {
        if (!m_frameStarted) {
            std::cerr << "Called Draw() while frame is not started!" << std::endl;
        }

        const auto &frame = m_swapChain->CurrentFrame();
        const auto &commandBuffer = frame.commandBuffers.at(vk::QueueFlagBits::eGraphics);
        const auto &renderPass = m_swapChain->RenderPass();

        renderPass.Begin(commandBuffer, frame.imageIndex);
        drawFunc(commandBuffer);
        renderPass.End(commandBuffer);
    }

    bool Renderer::EndFrame() {
        if (!m_frameStarted) {
            std::cerr << "Called EndFrame() while frame is not started!" << std::endl;
        }

        if (m_window.IsPaused()) {
            return false;
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
