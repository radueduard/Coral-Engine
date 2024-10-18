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

        std::cout << "Swap chain created with " << m_swapChainImages.size() << " images" << std::endl;

        m_extent = swapExtent;

        const auto colorAttachment = vk::AttachmentDescription()
            .setFormat(surfaceFormat.format)
            .setSamples(vk::SampleCountFlagBits::e1)
            .setLoadOp(vk::AttachmentLoadOp::eClear)
            .setStoreOp(vk::AttachmentStoreOp::eStore)
            .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
            .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
            .setInitialLayout(vk::ImageLayout::eUndefined)
            .setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

        constexpr auto colorAttachmentRef = vk::AttachmentReference()
            .setAttachment(0)
            .setLayout(vk::ImageLayout::eColorAttachmentOptimal);

        const auto subpass = vk::SubpassDescription()
            .setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
            .setColorAttachments(colorAttachmentRef);

        const auto dependencies = {
            vk::SubpassDependency()
                .setSrcSubpass(VK_SUBPASS_EXTERNAL)
                .setDstSubpass(0)
                .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
                .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
                .setSrcAccessMask(vk::AccessFlagBits::eNoneKHR)
                .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite),
            vk::SubpassDependency()
                .setSrcSubpass(0)
                .setDstSubpass(VK_SUBPASS_EXTERNAL)
                .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
                .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
                .setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
                .setDstAccessMask(vk::AccessFlagBits::eNoneKHR),
        };

        const auto renderPassCreateInfo = vk::RenderPassCreateInfo()
            .setAttachments(colorAttachment)
            .setSubpasses(subpass)
            .setDependencies(dependencies);

        m_renderPass = (*m_device).createRenderPass(renderPassCreateInfo);

        m_frames.reserve(imageCount);
        for (uint32_t i = 0; i < imageCount; i++) {
            constexpr auto fenceCreateInfo = vk::FenceCreateInfo()
                .setFlags(vk::FenceCreateFlagBits::eSignaled);
            constexpr auto semaphoreCreateInfo = vk::SemaphoreCreateInfo();

            vk::Fence inFlight = (*m_device).createFence(fenceCreateInfo);
            vk::Semaphore imageAvailable = (*m_device).createSemaphore(semaphoreCreateInfo);
            vk::Semaphore renderFinished = (*m_device).createSemaphore(semaphoreCreateInfo);

            const auto image = m_swapChainImages[i];

            constexpr auto components = vk::ComponentMapping()
                .setR(vk::ComponentSwizzle::eIdentity)
                .setG(vk::ComponentSwizzle::eIdentity)
                .setB(vk::ComponentSwizzle::eIdentity)
                .setA(vk::ComponentSwizzle::eIdentity);

            constexpr auto subresourceRange = vk::ImageSubresourceRange()
                .setAspectMask(vk::ImageAspectFlagBits::eColor)
                .setBaseMipLevel(0)
                .setLevelCount(1)
                .setBaseArrayLayer(0)
                .setLayerCount(1);

            const auto imageViewCreateInfo = vk::ImageViewCreateInfo()
                .setImage(image)
                .setViewType(vk::ImageViewType::e2D)
                .setFormat(surfaceFormat.format)
                .setComponents(components)
                .setSubresourceRange(subresourceRange);
            const auto imageView = (*m_device).createImageView(imageViewCreateInfo);

            const auto framebufferCreateInfo = vk::FramebufferCreateInfo()
                .setRenderPass(m_renderPass)
                .setAttachments(imageView)
                .setWidth(m_extent.width)
                .setHeight(m_extent.height)
                .setLayers(1);
            const auto framebuffer = (*m_device).createFramebuffer(framebufferCreateInfo);

            const auto commandBufferAllocateInfo = vk::CommandBufferAllocateInfo()
                .setCommandPool(m_device.CommandPool(Core::Queue::Type::Graphics))
                .setLevel(vk::CommandBufferLevel::ePrimary)
                .setCommandBufferCount(1);

            const auto commandBuffer = (*m_device).allocateCommandBuffers(commandBufferAllocateInfo)[0];

            Frame frame = {
                .imageIndex = i,
                .inFlight = inFlight,
                .imageAvailable = imageAvailable,
                .renderFinished = renderFinished,
                .image = image,
                .imageView = imageView,
                .framebuffer = framebuffer,
                .commandBuffers = {
                    { vk::QueueFlagBits::eGraphics, commandBuffer },
                }
            };

            m_frames.emplace_back(frame);
        }
    }

    SwapChain::~SwapChain() {
        (*m_device).waitIdle();
        (*m_device).destroyRenderPass(m_renderPass);
        for (auto &frame : m_frames) {
            (*m_device).destroyFence(frame.inFlight);
            (*m_device).destroySemaphore(frame.imageAvailable);
            (*m_device).destroySemaphore(frame.renderFinished);

            (*m_device).destroyFramebuffer(frame.framebuffer);
            (*m_device).destroyImageView(frame.imageView);

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

        // TODO: Check if this is better than resetting the fence right before submitting the command buffer
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

        // TODO: Check if this is better than resetting the fence right after acquiring the image
        // if (const auto result = (*m_device).resetFences(1, &m_frames[m_currentFrame].inFlight);
        //     result != vk::Result::eSuccess) {
        //     std::cerr << "Failed to reset fence: " << vk::to_string(result) << std::endl;
        // }

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
        } catch (const vk::OutOfDateKHRError &e) {
            std::cerr << e.what() << std::endl;
            return vk::Result::eErrorOutOfDateKHR;
        }
    }
}