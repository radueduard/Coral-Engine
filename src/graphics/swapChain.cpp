//
// Created by radue on 10/14/2024.
//

#include "swapChain.h"
#include "renderer.h"

#include <iostream>

namespace Graphics {
    vk::SurfaceFormatKHR SwapChain::ChooseSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &availableFormats) {
        for (const auto &availableFormat : availableFormats) {
            if (availableFormat.format == vk::Format::eR8G8B8A8Srgb && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
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
        actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));
        return actualExtent;
    }

    SwapChain::SwapChain(const vk::Extent2D extent, const Settings& settings, std::unique_ptr<SwapChain> oldSwapChain)
        : m_imageCount(settings.imageCount)
    {
        const auto &m_device = Core::Device::Get();
        (*m_device).waitIdle();
        m_device.QuerySurfaceCapabilities();

        const vk::SurfaceFormatKHR surfaceFormat = ChooseSurfaceFormat(Core::Runtime::Get().PhysicalDevice().SurfaceFormats());
        const vk::PresentModeKHR presentMode = ChoosePresentMode(Core::Runtime::Get().PhysicalDevice().SurfacePresentModes());
        m_extent = ChooseExtent(Core::Runtime::Get().PhysicalDevice().SurfaceCapabilities(), extent);

        if (oldSwapChain) {
            m_presentQueue = oldSwapChain->m_presentQueue;
        } else {
            m_presentQueue = m_device.RequestPresentQueue().value();
        }

        CreateSwapChain(surfaceFormat, presentMode, settings, oldSwapChain);
        CreateRenderPass(surfaceFormat, settings, oldSwapChain);

        oldSwapChain.reset();
    }

    void SwapChain::CreateSwapChain(const vk::SurfaceFormatKHR &surfaceFormat, const vk::PresentModeKHR &presentMode, const Settings& settings, const std::unique_ptr<SwapChain> &oldSwapChain) {
        auto queueFamilyIndices = settings.queueFamilyIndices;
        queueFamilyIndices.push_back(m_presentQueue->familyIndex);

        const auto& device = Core::Device::Get();

        auto createInfo = vk::SwapchainCreateInfoKHR()
            .setSurface(Core::Runtime::Get().PhysicalDevice().Surface())
            .setMinImageCount(settings.imageCount)
            .setImageFormat(surfaceFormat.format)
            .setImageColorSpace(surfaceFormat.colorSpace)
            .setImageExtent(m_extent)
            .setImageArrayLayers(1)
            .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
            .setImageSharingMode(vk::SharingMode::eExclusive)
            .setQueueFamilyIndices(queueFamilyIndices)
            .setPreTransform(Core::Runtime::Get().PhysicalDevice().SurfaceCapabilities().currentTransform)
            .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
            .setPresentMode(presentMode)
            .setClipped(true);

        if (oldSwapChain) {
            createInfo.setOldSwapchain(oldSwapChain->m_swapChain);
            m_imageIndex = oldSwapChain->m_imageIndex;
        }
        m_swapChain = (*device).createSwapchainKHR(createInfo);
    }

    void SwapChain::CreateRenderPass(const vk::SurfaceFormatKHR &surfaceFormat, const Settings& settings, const std::unique_ptr<SwapChain> &oldSwapChain) {
        const auto &device = Core::Device::Get();
        const auto swapChainImageHandles = (*device).getSwapchainImagesKHR(m_swapChain);
        std::vector<std::unique_ptr<Memory::Image>> swapChainImages;
        swapChainImages.reserve(swapChainImageHandles.size());
        for (const auto &image : swapChainImageHandles) {
            swapChainImages.emplace_back(Memory::Image::Builder()
                .Image(image)
                .Format(surfaceFormat.format)
                .Extent({ m_extent.width, m_extent.height, 1 })
                .UsageFlags(vk::ImageUsageFlagBits::eColorAttachment)
                .MipLevels(1)
                .LayersCount(1)
                .InitialLayout(vk::ImageLayout::eUndefined)
                .Build());
        }

        auto colorAttachment = RenderPass::Attachment {
            .description = vk::AttachmentDescription()
                .setFormat(surfaceFormat.format)
                .setSamples(settings.sampleCount)
                .setLoadOp(vk::AttachmentLoadOp::eClear)
                .setStoreOp(vk::AttachmentStoreOp::eStore)
                .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
                .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
                .setInitialLayout(vk::ImageLayout::eUndefined)
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

        if (settings.sampleCount == vk::SampleCountFlagBits::e1) {
            colorAttachment.images = std::move(swapChainImages);

            auto subpass = vk::SubpassDescription()
                .setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
                .setColorAttachmentCount(1)
                .setPColorAttachments(&colorAttachment.reference);

            m_renderPass = RenderPass::Builder()
               .ImageCount(settings.imageCount)
               .OutputImageIndex(0)
               .Extent(m_extent)
               .Attachment(0, std::move(colorAttachment))
               .Subpass(subpass)
               .Dependency(dependency1)
               .Dependency(dependency2)
               .Build();
        } else {
            // colorAttachment.description.finalLayout = vk::ImageLayout::eColorAttachmentOptimal;

            RenderPass::Attachment resolveAttachment = {
                .description = vk::AttachmentDescription()
                    .setFormat(surfaceFormat.format)
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

            for (uint32_t i = 0; i < settings.imageCount; i++) {
                colorAttachment.images.emplace_back(Memory::Image::Builder()
                    .Format(surfaceFormat.format)
                    .Extent({ m_extent.width, m_extent.height, 1 })
                    .UsageFlags(vk::ImageUsageFlagBits::eColorAttachment)
                    .MipLevels(1)
                    .LayersCount(1)
                    .SampleCount(settings.sampleCount)
                    .InitialLayout(vk::ImageLayout::eUndefined)
                    .Build());
            }

            resolveAttachment.images = std::move(swapChainImages);

            auto subpass = vk::SubpassDescription()
                .setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
                .setColorAttachmentCount(1)
                .setPColorAttachments(&colorAttachment.reference)
                .setPResolveAttachments(&resolveAttachment.reference);

            m_renderPass = RenderPass::Builder()
                .ImageCount(settings.imageCount)
                .OutputImageIndex(1)
                .Extent(m_extent)
                .Attachment(0, std::move(colorAttachment))
                .Attachment(1, std::move(resolveAttachment))
                .Subpass(subpass)
                .Dependency(dependency1)
                .Dependency(dependency2)
                .Build();

            if (oldSwapChain) {
                uint32_t i = 0;
                for (auto& subpassPrograms : oldSwapChain->m_renderPass->Programs()) {
                    for (auto program : subpassPrograms) {
                        m_renderPass->AddProgram(program, i);
                    }
                    i++;
                }
            }
        }
    }

    SwapChain::~SwapChain() {
        (*Core::Device::Get()).waitIdle();
        if (m_swapChain) {
            (*Core::Device::Get()).destroySwapchainKHR(m_swapChain);
        }
    }

    vk::Result SwapChain::Acquire(const mgv::Frame &frame) {
        try {
            const auto result = (*Core::Device::Get()).acquireNextImageKHR(
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
        m_renderPass->Begin(commandBuffer, mgv::Renderer::CurrentFrame().imageIndex);
        m_renderPass->Draw(commandBuffer);
        GUI::Manager::Render(commandBuffer);
        m_renderPass->End(commandBuffer);
    }

    vk::Result SwapChain::Present(const mgv::Frame &frame) {
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
            return (**m_presentQueue).presentKHR(presentInfo);
        } catch (const vk::OutOfDateKHRError &) {
            return vk::Result::eErrorOutOfDateKHR;
        }
    }
}
