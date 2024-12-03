//
// Created by radue on 10/14/2024.
//

#pragma once

#include <vulkan/vulkan.hpp>

#include "../core/device.h"
#include "renderPass.h"

namespace mgv {
    struct Frame;
    class Renderer;
}

namespace Graphics {
    class SwapChain {
    public:
        struct Settings {
            uint32_t minImageCount;
            uint32_t imageCount;
            vk::SampleCountFlagBits sampleCount;
            std::vector<uint32_t> queueFamilyIndices;
        };

        SwapChain(vk::Extent2D extent, const Settings& settings, std::unique_ptr<SwapChain> oldSwapChain = nullptr);
        ~SwapChain();

        vk::SwapchainKHR operator *() const { return m_swapChain; }
        [[nodiscard]] const vk::Extent2D &Extent() const { return m_extent; }
        [[nodiscard]] uint32_t ImageCount() const { return m_imageCount; }
        [[nodiscard]] RenderPass &RenderPass() const { return *m_renderPass; }

        vk::Result Acquire(const mgv::Frame &frame);

        void Render(vk::CommandBuffer commandBuffer) const;

        vk::Result Present(const mgv::Frame &frame);
    private:
        vk::SwapchainKHR m_swapChain;
        vk::Extent2D m_extent;
        uint32_t m_imageCount;
        Core::Queue *m_presentQueue;

        std::unique_ptr<Graphics::RenderPass> m_renderPass;

        uint32_t m_imageIndex = 0;

        void CreateSwapChain(const vk::SurfaceFormatKHR &surfaceFormat, const vk::PresentModeKHR &presentMode, const Settings& settings, const std::unique_ptr<SwapChain> &oldSwapChain);
        void CreateRenderPass(const vk::SurfaceFormatKHR &surfaceFormat, const Settings& settings, const std::unique_ptr<SwapChain> &oldSwapChain);

        static vk::SurfaceFormatKHR ChooseSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &availableFormats);
        static vk::PresentModeKHR ChoosePresentMode(const std::vector<vk::PresentModeKHR> &availablePresentModes);
        static vk::Extent2D ChooseExtent(const vk::SurfaceCapabilitiesKHR &capabilities, vk::Extent2D extent);
    };
}
