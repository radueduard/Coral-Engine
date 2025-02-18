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

#include "../core/scheduler.h"
#include "gui/manager.h"

namespace Graphics {
    vk::SurfaceFormatKHR SwapChain::ChooseSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &availableFormats) {
        // for (const auto &availableFormat : availableFormats) {
        //     std::cout << "Available format: " << vk::to_string(availableFormat.format) << std::endl;
        //     std::cout << "Available color space: " << vk::to_string(availableFormat.colorSpace) << std::endl << std::endl;
        // }

        for (const auto &availableFormat : availableFormats) {
            if (availableFormat.format == vk::Format::eA2B10G10R10UnormPack32 && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            // if (availableFormat.format == vk::Format::eB8G8R8A8Unorm && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
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

    vk::Extent2D SwapChain::ChooseExtent(const vk::SurfaceCapabilitiesKHR &capabilities, const vk::Extent2D extent) {
        if (capabilities.currentExtent.width != UINT32_MAX) {
            return capabilities.currentExtent;
        }
        vk::Extent2D actualExtent = extent;
        actualExtent.width = std::max<uint32_t>(capabilities.minImageExtent.width, std::min<uint32_t>(capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height = std::max<uint32_t>(capabilities.minImageExtent.height, std::min<uint32_t>(capabilities.maxImageExtent.height, actualExtent.height));
        return actualExtent;
    }

    SwapChain::SwapChain(const Core::Device& device, const CreateInfo& createInfo)
        : m_device(device), m_extent(createInfo.extent), m_sampleCount(createInfo.sampleCount), m_minImageCount(createInfo.minImageCount), m_imageCount(createInfo.imageCount)
    {
        m_device.Handle().waitIdle();
        m_queueFamilyIndices = createInfo.queueFamilyIndices;
        m_presentQueue = m_device.RequestPresentQueue().value();
        m_queueFamilyIndices.emplace_back(m_presentQueue->familyIndex);

        CreateSwapChain();
        CreateRenderPass();
    }

    void SwapChain::CreateSwapChain() {
        const auto& physicalDevice = m_device.QuerySurfaceCapabilities();
        m_surfaceFormat = ChooseSurfaceFormat(physicalDevice.SurfaceFormats());
        m_presentMode = ChoosePresentMode(physicalDevice.SurfacePresentModes());
        m_extent = ChooseExtent(physicalDevice.SurfaceCapabilities(), m_extent);

        const vk::SwapchainKHR oldSwapChain = m_swapChain;

        const auto createInfo = vk::SwapchainCreateInfoKHR()
            .setSurface(physicalDevice.Surface())
            .setMinImageCount(m_imageCount)
            .setImageFormat(m_surfaceFormat.format)
            .setImageColorSpace(m_surfaceFormat.colorSpace)
            .setImageExtent(m_extent)
            .setImageArrayLayers(1)
            .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
            .setImageSharingMode(vk::SharingMode::eExclusive)
            .setQueueFamilyIndices(m_queueFamilyIndices)
            .setPreTransform(physicalDevice.SurfaceCapabilities().currentTransform)
            .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
            .setPresentMode(m_presentMode)
            .setOldSwapchain(oldSwapChain)
            .setClipped(true);
        m_swapChain = m_device.Handle().createSwapchainKHR(createInfo);

        if (oldSwapChain) {
            m_device.Handle().destroySwapchainKHR(oldSwapChain);
        }
    }

    void SwapChain::CreateRenderPass() {
        const auto swapChainImageHandles = m_device.Handle().getSwapchainImagesKHR(m_swapChain);

        m_swapChainImages.clear();
        m_swapChainImages.reserve(swapChainImageHandles.size());
        for (const auto &image : swapChainImageHandles) {
            m_swapChainImages.emplace_back(Memory::Image::Builder()
                .Image(image)
                .Format(m_surfaceFormat.format)
                .Extent({ m_extent.width, m_extent.height, 1 })
                .UsageFlags(vk::ImageUsageFlagBits::eColorAttachment)
                .MipLevels(1)
                .LayersCount(1)
                .InitialLayout(vk::ImageLayout::ePresentSrcKHR)
                .Build(m_device));
        }

        auto colorAttachment = RenderPass::Attachment {
            .description = vk::AttachmentDescription()
                .setFormat(m_surfaceFormat.format)
                .setSamples(m_sampleCount)
                .setLoadOp(vk::AttachmentLoadOp::eClear)
                .setStoreOp(vk::AttachmentStoreOp::eStore)
                .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
                .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
                .setInitialLayout(vk::ImageLayout::eColorAttachmentOptimal)
                .setFinalLayout(vk::ImageLayout::ePresentSrcKHR),
            .reference = vk::AttachmentReference()
                .setAttachment(0)
                .setLayout(vk::ImageLayout::eColorAttachmentOptimal),
            .clearValue = vk::ClearColorValue(std::array { 0.0f, 0.0f, 0.0f, 1.0f })
        };

        auto dependency1 = vk::SubpassDependency()
            .setSrcSubpass(vk::SubpassExternal)
            .setDstSubpass(0)
            .setSrcStageMask(vk::PipelineStageFlagBits::eTopOfPipe)
            .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
            .setSrcAccessMask(vk::AccessFlagBits::eNone)
            .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eColorAttachmentRead);

        auto dependency2 = vk::SubpassDependency()
            .setSrcSubpass(0)
            .setDstSubpass(vk::SubpassExternal)
            .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
            .setDstStageMask(vk::PipelineStageFlagBits::eBottomOfPipe)
            .setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eColorAttachmentRead)
            .setDstAccessMask(vk::AccessFlagBits::eNone);

        if (m_sampleCount == vk::SampleCountFlagBits::e1) {
            for (const auto& image : m_swapChainImages) {
                colorAttachment.images.emplace_back(image.get());
            }

            auto subpass = vk::SubpassDescription()
                .setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
                .setColorAttachmentCount(1)
                .setPColorAttachments(&colorAttachment.reference);

            m_renderPass = RenderPass::Builder()
               .ImageCount(m_imageCount)
               .OutputImageIndex(0)
               .Extent(m_extent)
               .Attachment(0, std::move(colorAttachment))
               .Subpass(subpass)
               .Dependency(dependency1)
               .Dependency(dependency2)
               .Build(m_device);
        } else {
            colorAttachment.description
                .setInitialLayout(vk::ImageLayout::eColorAttachmentOptimal)
                .setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal);

            RenderPass::Attachment resolveAttachment = {
                .description = vk::AttachmentDescription()
                    .setFormat(m_surfaceFormat.format)
                    .setSamples(vk::SampleCountFlagBits::e1)
                    .setLoadOp(vk::AttachmentLoadOp::eDontCare)
                    .setStoreOp(vk::AttachmentStoreOp::eStore)
                    .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
                    .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
                    .setInitialLayout(vk::ImageLayout::eUndefined)
                    .setFinalLayout(vk::ImageLayout::ePresentSrcKHR),
                .reference = vk::AttachmentReference()
                    .setAttachment(1)
                    .setLayout(vk::ImageLayout::eColorAttachmentOptimal),
                .clearValue = vk::ClearColorValue(std::array { 0.0f, 0.0f, 0.0f, 1.0f })
            };

            for (uint32_t i = 0; i < m_imageCount; i++) {
                auto msaaImage = Memory::Image::Builder()
                    .Format(m_surfaceFormat.format)
                    .Extent({ m_extent.width, m_extent.height, 1 })
                    .UsageFlags(vk::ImageUsageFlagBits::eColorAttachment)
                    .MipLevels(1)
                    .LayersCount(1)
                    .SampleCount(m_sampleCount)
                    .InitialLayout(vk::ImageLayout::eColorAttachmentOptimal)
                    .Build(m_device);

                colorAttachment.images.emplace_back(msaaImage.get());
                m_multiSampledImages.emplace_back(std::move(msaaImage));
            }

            for (const auto& image : m_swapChainImages) {
                resolveAttachment.images.emplace_back(image.get());
            }

            auto subpass = vk::SubpassDescription()
                .setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
                .setColorAttachmentCount(1)
                .setPColorAttachments(&colorAttachment.reference)
                .setPResolveAttachments(&resolveAttachment.reference);

            m_renderPass = RenderPass::Builder()
                .ImageCount(m_imageCount)
                .OutputImageIndex(1)
                .Extent(m_extent)
                .Attachment(0, std::move(colorAttachment))
                .Attachment(1, std::move(resolveAttachment))
                .Subpass(subpass)
                .Dependency(dependency1)
                .Dependency(dependency2)
                .Build(m_device);
        }
    }

    SwapChain::~SwapChain() {
        m_device.Handle().waitIdle();
        if (m_swapChain) {
            m_device.Handle().destroySwapchainKHR(m_swapChain);
        }
    }

    vk::SampleCountFlagBits SwapChain::SampleCount() const { return m_renderPass->SampleCount(); }

    void SwapChain::Resize(const vk::Extent2D newSize) {
        m_extent = newSize;
        m_device.Handle().waitIdle();
        CreateSwapChain();
        CreateRenderPass();
    }

    vk::Result SwapChain::Acquire(const Core::Frame &frame) {
        try {
            const auto result = m_device.Handle().acquireNextImageKHR(
                m_swapChain,
                UINT64_MAX,
                frame.imageAvailable);
            m_imageIndex = result.value;
            return result.result;
        } catch (const vk::OutOfDateKHRError &) {
            return vk::Result::eErrorOutOfDateKHR;
        }
    }

    void SwapChain::Render(const vk::CommandBuffer commandBuffer) const {
        m_renderPass->Begin(commandBuffer, m_imageIndex);
        m_renderPass->Draw(commandBuffer);
        GUI::Render(commandBuffer);
        m_renderPass->End(commandBuffer);
    }

    vk::Result SwapChain::Present(const Core::Frame &frame) {
        std::array waitSemaphores = {
            frame.renderFinished
        };

        std::array swapChains = {
            m_swapChain
        };

        const auto presentInfo = vk::PresentInfoKHR()
            .setWaitSemaphores(waitSemaphores)
            .setSwapchains(swapChains)
            .setImageIndices(m_imageIndex);

        try {
            return m_presentQueue->Handle().presentKHR(presentInfo);
        } catch (const vk::OutOfDateKHRError &) {
            return vk::Result::eErrorOutOfDateKHR;
        }
    }
}
