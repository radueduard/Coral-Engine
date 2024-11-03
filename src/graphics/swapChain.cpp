//
// Created by radue on 10/14/2024.
//

#include "swapChain.h"

#include <iostream>

namespace Graphics {
    vk::SurfaceFormatKHR SwapChain::ChooseSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &availableFormats) {
        for (const auto &availableFormat : availableFormats) {
            if (availableFormat.format == vk::Format::eB8G8R8A8Srgb && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
                return availableFormat;
            }
        }
        return availableFormats[0];
    }

    vk::PresentModeKHR SwapChain::ChoosePresentMode(const std::vector<vk::PresentModeKHR> &availablePresentModes) {
        for (const auto &availablePresentMode : availablePresentModes) {
            if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
                return availablePresentMode;
            }
        }
        return vk::PresentModeKHR::eFifo;
    }

    vk::Extent2D SwapChain::ChooseExtent(const vk::SurfaceCapabilitiesKHR &capabilities, const vk::Extent2D extent) {
        if (capabilities.currentExtent.width != UINT32_MAX) {
            return capabilities.currentExtent;
        }
        vk::Extent2D actualExtent = extent;
        actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));
        return actualExtent;
    }

    SwapChain::SwapChain(const Core::Device &device, const vk::Extent2D extent, std::unique_ptr<SwapChain> oldSwapChain)
        : m_device(device) {

        (*m_device).waitIdle();
        m_device.QuerySurfaceCapabilities();

        const vk::SurfaceFormatKHR surfaceFormat = ChooseSurfaceFormat(m_device.PhysicalDevice().SurfaceFormats());
        const vk::PresentModeKHR presentMode = ChoosePresentMode(m_device.PhysicalDevice().SurfacePresentModes());
        const vk::Extent2D swapExtent = ChooseExtent(m_device.PhysicalDevice().SurfaceCapabilities(), extent);

        const uint32_t imageCount = m_device.PhysicalDevice().SurfaceCapabilities().minImageCount;

        std::array queueFamilyIndices = {
            m_device.Queue(Core::Queue::Type::Graphics)->familyIndex,
            m_device.Queue(Core::Queue::Type::Present)->familyIndex
        };

        auto createInfo = vk::SwapchainCreateInfoKHR()
            .setSurface(m_device.PhysicalDevice().Surface())
            .setMinImageCount(imageCount)
            .setImageFormat(surfaceFormat.format)
            .setImageColorSpace(surfaceFormat.colorSpace)
            .setImageExtent(swapExtent)
            .setImageArrayLayers(1)
            .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
            .setImageSharingMode(vk::SharingMode::eConcurrent)
            .setQueueFamilyIndices(queueFamilyIndices)
            .setPreTransform(m_device.PhysicalDevice().SurfaceCapabilities().currentTransform)
            .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
            .setPresentMode(presentMode)
            .setClipped(true);

        if (oldSwapChain) {
            createInfo.setOldSwapchain(oldSwapChain->m_swapChain);
        }

        m_swapChain = (*m_device).createSwapchainKHR(createInfo);
        m_swapChainImages = (*m_device).getSwapchainImagesKHR(m_swapChain);

        if (oldSwapChain) {
            oldSwapChain.reset();
        }

        m_extent = swapExtent;

        auto attachment = RenderPass::Attachment {
            .description = vk::AttachmentDescription()
                .setFormat(surfaceFormat.format)
                .setSamples(vk::SampleCountFlagBits::e1)
                .setLoadOp(vk::AttachmentLoadOp::eClear)
                .setStoreOp(vk::AttachmentStoreOp::eStore)
                .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
                .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
                .setInitialLayout(vk::ImageLayout::eUndefined)
                .setFinalLayout(vk::ImageLayout::ePresentSrcKHR),
            .reference = vk::AttachmentReference()
                .setAttachment(0)
                .setLayout(vk::ImageLayout::eColorAttachmentOptimal),
            .images = {},
            .clearValue = vk::ClearColorValue(std::array { 0.0f, 0.0f, 0.0f, 1.0f })
        };

        for (const auto &image : m_swapChainImages) {
            attachment.images.emplace_back(Memory::Image::Builder()
                .Image(image)
                .Format(surfaceFormat.format)
                .Extent({ m_extent.width, m_extent.height, 1 })
                .UsageFlags(vk::ImageUsageFlagBits::eColorAttachment)
                .MipLevels(1)
                .LayersCount(1)
                .InitialLayout(vk::ImageLayout::eUndefined)
                .Build(m_device));
        }

        const auto subpass = vk::SubpassDescription()
            .setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
            .setColorAttachmentCount(1)
            .setPColorAttachments(&attachment.reference);

        const auto dependencies = std::vector {
            vk::SubpassDependency()
                .setSrcSubpass(vk::SubpassExternal)
                .setDstSubpass(0)
                .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
                .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
                .setSrcAccessMask(vk::AccessFlagBits::eNone)
                .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite),
            vk::SubpassDependency()
                .setSrcSubpass(0)
                .setDstSubpass(vk::SubpassExternal)
                .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
                .setDstStageMask(vk::PipelineStageFlagBits::eBottomOfPipe)
                .setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite)
                .setDstAccessMask(vk::AccessFlagBits::eNone)
            };

        m_renderPass = RenderPass::Builder()
            .ImageCount(imageCount)
            .OutputImageIndex(0)
            .Extent(m_extent)
            .Attachment(0, std::move(attachment))
            .Subpass(subpass)
            .Dependency(dependencies[0])
            .Dependency(dependencies[1])
            .Build(m_device);

        m_frames.reserve(imageCount);
        for (uint32_t i = 0; i < imageCount; i++) {
            constexpr auto fenceCreateInfo = vk::FenceCreateInfo()
                .setFlags(vk::FenceCreateFlagBits::eSignaled);
            constexpr auto semaphoreCreateInfo = vk::SemaphoreCreateInfo();

            vk::Fence inFlight = (*m_device).createFence(fenceCreateInfo);
            vk::Semaphore imageAvailable = (*m_device).createSemaphore(semaphoreCreateInfo);
            vk::Semaphore renderFinished = (*m_device).createSemaphore(semaphoreCreateInfo);

            const auto commandBufferAllocateInfo = vk::CommandBufferAllocateInfo()
                .setCommandPool(m_device.CommandPool(Core::Queue::Type::Graphics))
                .setLevel(vk::CommandBufferLevel::ePrimary)
                .setCommandBufferCount(1);

            const auto commandBuffer = (*m_device).allocateCommandBuffers(commandBufferAllocateInfo)[0];

             m_frames.emplace_back(Frame {
                .imageIndex = i,
                .inFlight = inFlight,
                .imageAvailable = imageAvailable,
                .renderFinished = renderFinished,
                 .image = Memory::Image::Builder()
                     .Image(m_swapChainImages[i])
                     .Format(surfaceFormat.format)
                     .Extent({ m_extent.width, m_extent.height, 1 })
                     .UsageFlags(vk::ImageUsageFlagBits::eColorAttachment)
                     .MipLevels(1)
                     .LayersCount(1)
                     .InitialLayout(vk::ImageLayout::eUndefined)
                     .Build(m_device),
                .commandBuffers = {
                    { vk::QueueFlagBits::eGraphics, commandBuffer },
                }
            });
        }
    }

    SwapChain::~SwapChain() {
        (*m_device).waitIdle();
        for (auto& frame : m_frames) {
            (*m_device).destroyFence(frame.inFlight);
            (*m_device).destroySemaphore(frame.imageAvailable);
            (*m_device).destroySemaphore(frame.renderFinished);

            (*m_device).freeCommandBuffers(m_device.CommandPool(Core::Queue::Type::Graphics), frame.commandBuffers[vk::QueueFlagBits::eGraphics]);
        }
        if (m_swapChain) {
            (*m_device).destroySwapchainKHR(m_swapChain);
        }
    }

    vk::Result SwapChain::Acquire() {
        if (const auto result = (*m_device).waitForFences(1, &m_frames[m_currentFrame].inFlight, VK_TRUE, UINT64_MAX);
            result != vk::Result::eSuccess) {
            std::cerr << "Failed to wait for fence: " << vk::to_string(result) << std::endl;
        }

        if (const auto result = (*m_device).resetFences(1, &m_frames[m_currentFrame].inFlight);
            result != vk::Result::eSuccess) {
            std::cerr << "Failed to reset fence: " << vk::to_string(result) << std::endl;
        }

        try {
            const auto result = (*m_device).acquireNextImageKHR(
                m_swapChain,
                UINT64_MAX,
                m_frames[m_currentFrame].imageAvailable);
            m_imageIndex = result.value;

            m_frames[m_currentFrame].commandBuffers[vk::QueueFlagBits::eGraphics].begin(vk::CommandBufferBeginInfo());
            return result.result;
        } catch (const vk::OutOfDateKHRError &e) {
            std::cerr << e.what() << std::endl;
            return vk::Result::eErrorOutOfDateKHR;
        }
    }

    void SwapChain::Submit() {
        auto graphicsCommandBuffer = m_frames[m_currentFrame].commandBuffers[vk::QueueFlagBits::eGraphics];
        graphicsCommandBuffer.end();

        std::array waitSemaphores = {
            m_frames[m_currentFrame].imageAvailable
        };

        vk::PipelineStageFlags waitStages = {
            vk::PipelineStageFlagBits::eColorAttachmentOutput
        };

        std::array signalSemaphores = {
            m_frames[m_currentFrame].renderFinished
        };

        const auto submitInfo = vk::SubmitInfo()
            .setCommandBuffers(graphicsCommandBuffer)
            .setWaitSemaphores(waitSemaphores)
            .setSignalSemaphores(signalSemaphores)
            .setWaitDstStageMask(waitStages);

        const auto& graphicsQueue = m_device.Queue(Core::Queue::Type::Graphics);

        graphicsQueue->queue.submit(submitInfo, m_frames[m_currentFrame].inFlight);
    }

    vk::Result SwapChain::Present() {
        std::array waitSemaphores = {
            m_frames[m_currentFrame].renderFinished
        };

        std::array swapChains = {
            m_swapChain
        };

        const auto presentInfo = vk::PresentInfoKHR()
            .setWaitSemaphores(waitSemaphores)
            .setSwapchains(swapChains)
            .setImageIndices(m_imageIndex);

        const auto& presentQueue = m_device.Queue(Core::Queue::Type::Present);
        try {
            const auto result = presentQueue->queue.presentKHR(presentInfo);
            m_currentFrame = (m_currentFrame + 1) % m_frames.size();
            return result;
        } catch (const vk::OutOfDateKHRError &_) {
            return vk::Result::eErrorOutOfDateKHR;
        }
    }
}