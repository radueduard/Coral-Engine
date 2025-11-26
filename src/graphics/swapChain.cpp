//
// Created by radue on 10/14/2024.
//

#include "swapChain.h"

#include <iostream>

#include "renderPass.h"
#include "core/device.h"
#include "core/physicalDevice.h"
#include "core/runtime.h"
#include "memory/image.h"

#include "core/scheduler.h"
#include "ecs/entity.h"
#include "gui/elements/popup.h"

namespace Coral::Graphics {
    vk::SurfaceFormatKHR SwapChain::ChooseSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &availableFormats) {
        for (const auto &availableFormat : availableFormats) {
            // if (availableFormat.format == vk::Format::eA2B10G10R10UnormPack32 && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            if (availableFormat.format == vk::Format::eB8G8R8A8Unorm && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            // if (availableFormat.format == vk::Format::eR8G8B8A8Srgb && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
                return availableFormat;
            }
        }
        std::cerr << "Failed to find suitable surface format" << std::endl;
        return availableFormats[0];
    }

    vk::PresentModeKHR SwapChain::ChoosePresentMode(const std::vector<vk::PresentModeKHR> &availablePresentModes) {
        for (const auto &availablePresentMode : availablePresentModes) {
            if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
                return availablePresentMode;
            }
        }
        std::cerr << "Failed to find suitable present mode" << std::endl;
        return vk::PresentModeKHR::eFifo;
    }

    Math::Vector2<u32> SwapChain::ChooseExtent(const vk::SurfaceCapabilitiesKHR &capabilities, const Math::Vector2<u32>& extent) {
        if (capabilities.currentExtent.width != UINT32_MAX) {
            return Math::Vector2<u32>(capabilities.currentExtent);
        }
        Math::Vector2<uint32_t> actualExtent = extent;
        actualExtent.x = std::max<uint32_t>(capabilities.minImageExtent.width, std::min<uint32_t>(capabilities.maxImageExtent.width, actualExtent.x));
        actualExtent.y = std::max<uint32_t>(capabilities.minImageExtent.height, std::min<uint32_t>(capabilities.maxImageExtent.height, actualExtent.y));
        return actualExtent;
    }

    SwapChain::SwapChain(const CreateInfo& createInfo)
        : m_sampleCount(createInfo.sampleCount), m_minImageCount(createInfo.minImageCount), m_imageCount(createInfo.imageCount)
    {
        m_extent = Math::Vector2<uint32_t>(1);

        Context::Device()->waitIdle();
        m_presentQueue = Context::Device().RequestPresentQueue();
        CreateSwapChain();
    }

    void SwapChain::CreateSwapChain() {
        const auto& physicalDevice =Context::Device().QuerySurfaceCapabilities();
        m_surfaceFormat = ChooseSurfaceFormat(physicalDevice.SurfaceFormats());
        m_presentMode = ChoosePresentMode(physicalDevice.SurfacePresentModes());
        m_extent = ChooseExtent(physicalDevice.SurfaceCapabilities(), m_extent);

        const vk::SwapchainKHR oldSwapChain = m_handle;
        const auto queueFamilyIndices = std::array { m_presentQueue->Family().Index() };

        const auto createInfo = vk::SwapchainCreateInfoKHR()
            .setSurface(physicalDevice.Surface())
            .setMinImageCount(m_imageCount)
            .setImageFormat(m_surfaceFormat.format)
            .setImageColorSpace(m_surfaceFormat.colorSpace)
            .setImageExtent({ static_cast<u32>(m_extent.x), static_cast<u32>(m_extent.y) })
            .setImageArrayLayers(1)
            .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst)
            .setImageSharingMode(vk::SharingMode::eExclusive)
            .setQueueFamilyIndices(queueFamilyIndices)
            .setPreTransform(physicalDevice.SurfaceCapabilities().currentTransform)
            .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
            .setPresentMode(m_presentMode)
            .setOldSwapchain(oldSwapChain)
            .setClipped(true);
        m_handle = Context::Device()->createSwapchainKHR(createInfo);

        if (oldSwapChain) {
            Context::Device()->destroySwapchainKHR(oldSwapChain);
        }

        const auto swapChainImageHandles = Context::Device()->getSwapchainImagesKHR(m_handle);
        m_swapChainImages.clear();
        m_swapChainImages.reserve(swapChainImageHandles.size());
        for (const auto &image : swapChainImageHandles) {
            m_swapChainImages.emplace_back(Memory::Image::Builder()
                .Image(image)
                .Format(m_surfaceFormat.format)
                .Extent({ static_cast<u32>(m_extent.x), static_cast<u32>(m_extent.y), 1u })
                .UsageFlags(vk::ImageUsageFlagBits::eColorAttachment)
                .MipLevels(1)
                .LayersCount(1)
                .InitialLayout(vk::ImageLayout::ePresentSrcKHR)
                .Build());
        }
    }

    SwapChain::~SwapChain() {
        Context::Device()->waitIdle();
        if (m_handle) {
            Context::Device()->destroySwapchainKHR(m_handle);
        }
    }

    std::vector<Memory::Image *> SwapChain::SwapChainImages() const {
        return m_swapChainImages
            | std::views::transform([](const auto& image) { return image.get(); })
            | std::ranges::to<std::vector<Memory::Image*>>();
    }

    void SwapChain::Resize(const Math::Vector2<f32>& newSize) {
        m_extent = newSize;
        Context::Device()->waitIdle();
        CreateSwapChain();
    }

    vk::Result SwapChain::Acquire(const Core::Frame &frame) {
        try {
            const auto result = Context::Device()->acquireNextImageKHR(
                m_handle,
                UINT64_MAX,
                frame.ImageAvailable());
            m_imageIndex = result.value;
            return result.result;
        } catch (const vk::OutOfDateKHRError &) {
            return vk::Result::eErrorOutOfDateKHR;
        }
    }

    vk::Result SwapChain::Present(const Core::Frame &frame) {
        std::array waitSemaphores = {
        	frame.FinalImageTransferCommandBuffer().SignalSemaphore()
        };

        std::array swapChains = {
            m_handle
        };

        const auto presentInfo = vk::PresentInfoKHR()
            .setWaitSemaphores(waitSemaphores)
            .setSwapchains(swapChains)
            .setImageIndices(m_imageIndex);

        try {
        	return frame.Queue()->presentKHR(presentInfo);
        } catch (const vk::OutOfDateKHRError &) {
            return vk::Result::eErrorOutOfDateKHR;
        }
    }
}
