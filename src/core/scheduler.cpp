//
// Created by radue on 10/14/2024.
//

#include "scheduler.h"

#include <iostream>
#include <ranges>

#include "graphics/renderPass.h"
#include "gui/manager.h"
#include "memory/descriptor/pool.h"
#include "project/renderGraph.h"

namespace Core {
    Frame::Frame(const uint32_t imageIndex)
        : m_imageIndex(imageIndex) {
        m_imageAvailable = Core::GlobalDevice()->createSemaphore({});
        m_inFlightFence = Core::GlobalDevice()->createFence(vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled));
        m_readyToPresent = Core::GlobalDevice()->createSemaphore({});
    }

    Frame::~Frame() {
        Core::GlobalDevice()->destroySemaphore(m_imageAvailable);
        Core::GlobalDevice()->destroySemaphore(m_readyToPresent);
        Core::GlobalDevice()->destroyFence(m_inFlightFence);
    }

    Scheduler::Scheduler(const CreateInfo& createInfo)
        : m_window(createInfo.window), m_runtime(createInfo.runtime), m_imageCount(createInfo.imageCount)
    {
        CreateFrames();

        const auto swapChainCreateInfo = Graphics::SwapChain::CreateInfo {
            .minImageCount = createInfo.minImageCount,
            .imageCount = m_imageCount,
            .sampleCount = createInfo.multiSampling,
        };
        m_swapChain = std::make_unique<Graphics::SwapChain>(swapChainCreateInfo);

        CreateDescriptorPool();

        const auto renderGraphCreateInfo = Project::RenderGraph::CreateInfo {
            .window = createInfo.window,
            .runtime = createInfo.runtime,
            .frameCount = m_imageCount,
        };

        m_renderGraph = std::make_unique<Project::RenderGraph>(renderGraphCreateInfo);
    }

    Scheduler::~Scheduler() {
        Core::GlobalDevice()->waitIdle();
    }

    void Scheduler::CreateDescriptorPool() {
        m_descriptorPool = Memory::Descriptor::Pool::Builder()
            .AddPoolSize(vk::DescriptorType::eUniformBuffer, 100)
            .AddPoolSize(vk::DescriptorType::eStorageBuffer, 100)
            .AddPoolSize(vk::DescriptorType::eCombinedImageSampler, 100)
            .AddPoolSize(vk::DescriptorType::eStorageImage, 100)
            .PoolFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet)
            .MaxSets(100)
            .Build();
    }

    void Scheduler::CreateFrames() {
        m_frames.reserve(m_imageCount);
        for (uint32_t i = 0; i < m_imageCount; i++) {
            m_frames.emplace_back(std::make_unique<Frame>(i));
        }
    }

    void Scheduler::Update(const float deltaTime) {
        m_renderGraph->Update(deltaTime);

        // m_renderGraph->Resize(m_window.Extent());
    }

    void Scheduler::Draw() {
        const auto& currentFrame = CurrentFrame();

        const auto& fence = currentFrame.InFlightFence();
        if (const auto result = Core::GlobalDevice()->waitForFences(1, &fence, vk::True, UINT64_MAX); result != vk::Result::eSuccess) {
            throw std::runtime_error("Failed to wait fence: " + vk::to_string(result));
        }

        if (const auto result = Core::GlobalDevice()->resetFences(1, &fence); result != vk::Result::eSuccess) {
            throw std::runtime_error("Failed to reset fence: " + vk::to_string(result));
        }

        if (const auto result = m_swapChain->Acquire(currentFrame);
            result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR) {
            m_swapChain->Resize(m_window.Extent());
            return;
        }

        m_renderGraph->Execute(currentFrame);

        Core::GlobalDevice().RunSingleTimeCommand([&](const Core::CommandBuffer& commandBuffer) {
            const Memory::Image& outputImage = m_renderGraph->OutputImage(currentFrame.ImageIndex());
            const Memory::Image& swapChainImage = *m_swapChain->SwapChainImages()[currentFrame.ImageIndex()];

            vk::ImageMemoryBarrier outputImageBarrier = vk::ImageMemoryBarrier()
                .setImage(*outputImage)
                .setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
                .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead)
                .setOldLayout(vk::ImageLayout::eColorAttachmentOptimal)
                .setNewLayout(vk::ImageLayout::eTransferSrcOptimal)
                .setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));

            commandBuffer->pipelineBarrier(
                vk::PipelineStageFlagBits::eAllGraphics,
                vk::PipelineStageFlagBits::eAllCommands,
                vk::DependencyFlags(),
                nullptr,
                nullptr,
                { outputImageBarrier }
            );

            const auto imageResolve = vk::ImageResolve()
                .setSrcSubresource(vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1))
                .setSrcOffset(vk::Offset3D(0, 0, 0))
                .setDstSubresource(vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1))
                .setDstOffset(vk::Offset3D(0, 0, 0))
                .setExtent(vk::Extent3D(m_window.Extent().width, m_window.Extent().height, 1));

            commandBuffer->resolveImage(
                *outputImage,
                vk::ImageLayout::eTransferSrcOptimal,
                *swapChainImage,
                vk::ImageLayout::eTransferDstOptimal,
                imageResolve
            );

            outputImageBarrier
                .setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentRead)
                .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
                .setOldLayout(vk::ImageLayout::eTransferSrcOptimal)
                .setNewLayout(vk::ImageLayout::eColorAttachmentOptimal);

            commandBuffer->pipelineBarrier(
                vk::PipelineStageFlagBits::eAllCommands,
                vk::PipelineStageFlagBits::eAllCommands,
                vk::DependencyFlags(),
                nullptr,
                nullptr,
                { outputImageBarrier }
            );
        },
        vk::QueueFlagBits::eGraphics,
        currentFrame.InFlightFence(),
        m_renderGraph->RenderFinished(currentFrame.ImageIndex()),
        currentFrame.ReadyToPresent());

        if (const auto result = m_swapChain->Present(currentFrame);
            result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR) {
            m_swapChain->Resize(m_window.Extent());
            return;
        }
        m_currentFrame = (m_currentFrame + 1) % m_imageCount;

    }
}
