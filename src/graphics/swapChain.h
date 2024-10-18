//
// Created by radue on 10/14/2024.
//

#pragma once

#include <unordered_map>
#include <vulkan/vulkan.hpp>

#include "../core/device.h"

namespace Graphics {
    class SwapChain {
    public:
        struct Frame {
            uint32_t imageIndex;

            vk::Fence inFlight;
            vk::Semaphore imageAvailable;
            vk::Semaphore renderFinished;

            vk::Image image;
            vk::ImageView imageView;
            vk::Framebuffer framebuffer;

            std::unordered_map<vk::QueueFlagBits, vk::CommandBuffer> commandBuffers;
        };

        SwapChain(const Core::Device &device, vk::Extent2D extent, std::unique_ptr<SwapChain> oldSwapChain = nullptr);
        ~SwapChain();

        vk::SwapchainKHR operator *() const { return m_swapChain; }
        vk::SwapchainKHR operator ->() const { return m_swapChain; }
        [[nodiscard]] const vk::Extent2D &Extent() const { return m_extent; }
        [[nodiscard]] const Frame &CurrentFrame() const { return m_frames[m_currentFrame]; }
        [[nodiscard]] const vk::RenderPass &RenderPass() const { return m_renderPass; }

        vk::Result Acquire();
        void Submit();
        vk::Result Present();
    private:
        const Core::Device &m_device;
        vk::SwapchainKHR m_swapChain;
        std::vector<vk::Image> m_swapChainImages;
        vk::Extent2D m_extent;

        uint32_t m_currentFrame = 0;
        std::vector<Frame> m_frames;
        vk::RenderPass m_renderPass;

        uint32_t m_imageIndex = 0;

        static vk::SurfaceFormatKHR ChooseSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &availableFormats);
        static vk::PresentModeKHR ChoosePresentMode(const std::vector<vk::PresentModeKHR> &availablePresentModes);
        static vk::Extent2D ChooseExtent(const vk::SurfaceCapabilitiesKHR &capabilities, const vk::Extent2D extent);
    };
}
