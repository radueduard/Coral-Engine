//
// Created by radue on 10/14/2024.
//

#pragma once

#include "core/device.h"
#include "math/vector.h"

namespace Coral::Memory {
    class Image;
}

namespace Coral::Core {
    class Frame;
    class Scheduler;
    class Queue;
}

namespace Coral::Graphics {
    class RenderPass;
}

namespace Coral::Graphics {
    class SwapChain : public EngineWrapper<vk::SwapchainKHR> {
    public:
        struct CreateInfo {
            uint32_t minImageCount;
            uint32_t imageCount;
            vk::SampleCountFlagBits sampleCount;
        };

        explicit SwapChain(const CreateInfo& createInfo);
        ~SwapChain() override;

        [[nodiscard]] const Math::Vector2<f32> &Extent() const { return m_extent; }
        [[nodiscard]] uint32_t MinImageCount() const { return m_minImageCount; }
        [[nodiscard]] uint32_t ImageCount() const { return m_imageCount; }
        [[nodiscard]] vk::SampleCountFlagBits SampleCount() const { return m_sampleCount; }
        [[nodiscard]] std::vector<Memory::Image*> SwapChainImages() const;
        [[nodiscard]] vk::Format ImageFormat() const { return m_surfaceFormat.format; }

        void Resize(const Math::Vector2<f32>& newSize);
        vk::Result Acquire(const Core::Frame &frame);
        vk::Result Present(const Core::Frame &frame);

    private:
        Math::Vector2<f32> m_extent;
        vk::SampleCountFlagBits m_sampleCount = vk::SampleCountFlagBits::e1;
        uint32_t m_minImageCount = 0;
        uint32_t m_imageCount = 0;
        uint32_t m_imageIndex = 0;
        vk::SurfaceFormatKHR m_surfaceFormat = {};
        vk::PresentModeKHR m_presentMode = {};

        std::unique_ptr<Core::Queue> m_presentQueue;
        std::vector<std::unique_ptr<Memory::Image>> m_swapChainImages;

        void CreateSwapChain();

        static vk::SurfaceFormatKHR ChooseSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &availableFormats);
        static vk::PresentModeKHR ChoosePresentMode(const std::vector<vk::PresentModeKHR> &availablePresentModes);
        static Math::Vector2<uint32_t> ChooseExtent(const vk::SurfaceCapabilitiesKHR &capabilities, const Math::Vector2<uint32_t>& extent);
    };
}
