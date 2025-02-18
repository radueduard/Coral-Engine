//
// Created by radue on 10/14/2024.
//

#pragma once

#include <vulkan/vulkan.hpp>

namespace Memory {
    class Image;
}

namespace Core {
    class Device;
    struct Frame;
    class Scheduler;
    struct Queue;
}

namespace Graphics {
    class RenderPass;
}

namespace Graphics {
    class SwapChain {
    public:
        struct CreateInfo {
            vk::Extent2D extent;
            uint32_t minImageCount;
            uint32_t imageCount;
            vk::SampleCountFlagBits sampleCount;
            std::vector<uint32_t> queueFamilyIndices;
        };

        explicit SwapChain(const Core::Device& device, const CreateInfo& createInfo);
        ~SwapChain();

        vk::SwapchainKHR operator *() const { return m_swapChain; }
        [[nodiscard]] const vk::Extent2D &Extent() const { return m_extent; }
        [[nodiscard]] uint32_t MinImageCount() const { return m_minImageCount; }
        [[nodiscard]] uint32_t ImageCount() const { return m_imageCount; }
        [[nodiscard]] vk::SampleCountFlagBits SampleCount() const;
        [[nodiscard]] RenderPass &RenderPass() const { return *m_renderPass; }

        void Resize(vk::Extent2D newSize);
        vk::Result Acquire(const Core::Frame &frame);
        void Render(vk::CommandBuffer commandBuffer) const;
        vk::Result Present(const Core::Frame &frame);

    private:
        const Core::Device &m_device;

        vk::Extent2D m_extent = {};
        vk::SampleCountFlagBits m_sampleCount = vk::SampleCountFlagBits::e1;
        uint32_t m_minImageCount = 0;
        uint32_t m_imageCount = 0;
        uint32_t m_imageIndex = 0;
        vk::SurfaceFormatKHR m_surfaceFormat = {};
        vk::PresentModeKHR m_presentMode = {};

        std::vector<unsigned> m_queueFamilyIndices;
        Core::Queue *m_presentQueue;

        std::unique_ptr<Graphics::RenderPass> m_renderPass;

        vk::SwapchainKHR m_swapChain = nullptr;
        std::vector<std::unique_ptr<Memory::Image>> m_swapChainImages;
        std::vector<std::unique_ptr<Memory::Image>> m_multiSampledImages;

        void CreateSwapChain();
        void CreateRenderPass();

        static vk::SurfaceFormatKHR ChooseSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &availableFormats);
        static vk::PresentModeKHR ChoosePresentMode(const std::vector<vk::PresentModeKHR> &availablePresentModes);
        static vk::Extent2D ChooseExtent(const vk::SurfaceCapabilitiesKHR &capabilities, vk::Extent2D extent);
    };
}
