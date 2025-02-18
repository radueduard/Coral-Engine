//
// Created by radue on 10/14/2024.
//

#include "scheduler.h"

#include <iostream>

#include "gui/manager.h"
#include "memory/descriptor/pool.h"

namespace Core {
    Scheduler::Scheduler(const CreateInfo& createInfo)
        : m_window(createInfo.window), m_runtime(createInfo.runtime), m_device(createInfo.device), m_imageCount(createInfo.imageCount)
    {
        CreateFrames();

        const auto swapChainCreateInfo = Graphics::SwapChain::CreateInfo {
            .extent = m_window.Extent(),
            .minImageCount = createInfo.minImageCount,
            .imageCount = m_imageCount,
            .sampleCount = createInfo.multiSampling,
            .queueFamilyIndices = { m_queues[vk::QueueFlagBits::eGraphics]->familyIndex },
        };
        m_swapChain = std::make_unique<Graphics::SwapChain>(m_device, swapChainCreateInfo);

        CreateDescriptorPool();

        const auto guiCreateInfo = GUI::CreateInfo {
            .window =  m_window,
            .runtime = m_runtime,
            .device = m_device,
            .scheduler = *this,
        };
        GUI::SetupContext(guiCreateInfo);
    }

    Scheduler::~Scheduler() {
        m_device.Handle().waitIdle();

        std::vector<CommandBuffer> commandBuffers;

        for (auto& frame : m_frames) {
            m_device.Handle().destroySemaphore(frame.imageAvailable);
            m_device.Handle().destroyFence(frame.depthPrePassInFlight);
            m_device.Handle().destroySemaphore(frame.depthPrePassFinished);
            m_device.Handle().destroyFence(frame.computeInFlight);
            m_device.Handle().destroySemaphore(frame.computeFinished);
            m_device.Handle().destroyFence(frame.graphicsInFlight);
            m_device.Handle().destroySemaphore(frame.graphicsFinished);
            m_device.Handle().destroyFence(frame.postProcessInFlight);
            m_device.Handle().destroySemaphore(frame.postProcessFinished);
            m_device.Handle().destroyFence(frame.renderInFlight);
            m_device.Handle().destroySemaphore(frame.renderFinished);

            commandBuffers.emplace_back(frame.commandBuffers.at(CommandBuffer::Usage::DepthPrePass));
            commandBuffers.emplace_back(frame.commandBuffers.at(CommandBuffer::Usage::Updates));
            commandBuffers.emplace_back(frame.commandBuffers.at(CommandBuffer::Usage::Graphics));
            commandBuffers.emplace_back(frame.commandBuffers.at(CommandBuffer::Usage::PostProcess));
            commandBuffers.emplace_back(frame.commandBuffers.at(CommandBuffer::Usage::Present));
        }
        m_device.FreeCommandBuffers(commandBuffers);
    }

    void Scheduler::CreateDescriptorPool() {
        m_descriptorPool = Memory::Descriptor::Pool::Builder()
            .AddPoolSize(vk::DescriptorType::eUniformBuffer, 100)
            .AddPoolSize(vk::DescriptorType::eStorageBuffer, 100)
            .AddPoolSize(vk::DescriptorType::eCombinedImageSampler, 100)
            .AddPoolSize(vk::DescriptorType::eStorageImage, 100)
            .PoolFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet)
            .MaxSets(100)
            .Build(m_device);
    }

    // void Scheduler::InitUI() {
    //     m_displayDescriptorSets = std::vector<vk::DescriptorSet>(m_imageCount);
    //     for (uint32_t i = 0; i < m_imageCount; i++) {
    //         const auto& outputImage = m_graphicsPass->RenderPass()->OutputImage(i);
    //         m_displayDescriptorSets[i] = ImGui_ImplVulkan_AddTexture(
    //             Memory::Sampler::Get(vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerAddressMode::eRepeat, vk::SamplerMipmapMode::eLinear),
    //             outputImage.ImageView(),
    //             static_cast<VkImageLayout>(vk::ImageLayout::eShaderReadOnlyOptimal));
    //     }
    // }
    //
    // void Scheduler::DestroyUI() {
    //     for (uint32_t i = 0; i < m_imageCount; i++) {
    //         ImGui_ImplVulkan_RemoveTexture(m_displayDescriptorSets[i]);
    //     }
    // }



    void Scheduler::CreateFrames() {
        m_frames.reserve(m_imageCount);
        for (uint32_t i = 0; i < m_imageCount; i++) {
            constexpr auto fenceCreateInfo = vk::FenceCreateInfo()
                .setFlags(vk::FenceCreateFlagBits::eSignaled);
            constexpr auto semaphoreCreateInfo = vk::SemaphoreCreateInfo();

            m_frames.emplace_back(Frame {
                .imageIndex = i,
                .imageAvailable = m_device.Handle().createSemaphore(semaphoreCreateInfo),
                .depthPrePassInFlight = m_device.Handle().createFence(fenceCreateInfo),
                .depthPrePassFinished = m_device.Handle().createSemaphore(semaphoreCreateInfo),
                .computeInFlight = m_device.Handle().createFence(fenceCreateInfo),
                .computeFinished = m_device.Handle().createSemaphore(semaphoreCreateInfo),
                .graphicsInFlight = m_device.Handle().createFence(fenceCreateInfo),
                .graphicsFinished = m_device.Handle().createSemaphore(semaphoreCreateInfo),
                .postProcessInFlight = m_device.Handle().createFence(fenceCreateInfo),
                .postProcessFinished = m_device.Handle().createSemaphore(semaphoreCreateInfo),
                .renderInFlight = m_device.Handle().createFence(fenceCreateInfo),
                .renderFinished = m_device.Handle().createSemaphore(semaphoreCreateInfo),
            });
        }

        Core::Queue* computeQueue = m_device.RequestQueue(vk::QueueFlagBits::eCompute).value();
        Core::Queue* graphicsQueue = m_device.RequestQueue(vk::QueueFlagBits::eGraphics).value();

        auto depthPrePassCommandBuffers = m_device.RequestCommandBuffers(graphicsQueue->familyIndex, m_imageCount);
        auto computeCommandBuffers = m_device.RequestCommandBuffers(computeQueue->familyIndex, m_imageCount);
        auto graphicsCommandBuffers = m_device.RequestCommandBuffers(graphicsQueue->familyIndex, m_imageCount);
        auto postProcessCommandBuffers = m_device.RequestCommandBuffers(computeQueue->familyIndex, m_imageCount);
        auto presentCommandBuffers = m_device.RequestCommandBuffers(graphicsQueue->familyIndex, m_imageCount);

        m_queues = {
            { vk::QueueFlagBits::eCompute, computeQueue },
            { vk::QueueFlagBits::eGraphics, graphicsQueue }
        };

        for (auto& frame : m_frames) {
            frame.commandBuffers = {
                { Core::CommandBuffer::Usage::DepthPrePass, depthPrePassCommandBuffers.at(frame.imageIndex) },
                { Core::CommandBuffer::Usage::Updates, computeCommandBuffers.at(frame.imageIndex) },
                { Core::CommandBuffer::Usage::Graphics, graphicsCommandBuffers.at(frame.imageIndex) },
                { Core::CommandBuffer::Usage::PostProcess, postProcessCommandBuffers.at(frame.imageIndex) },
                { Core::CommandBuffer::Usage::Present, presentCommandBuffers.at(frame.imageIndex) },
            };
        }
    }

    bool Scheduler::BeginFrame() {
        if (m_frameStarted) {
            std::cerr << "Called BeginFrame() while frame is already started!" << std::endl;
            return false;
        }

        if (m_window.IsPaused()) {
            return false;
        }

        const auto currentFrame = CurrentFrame();

        const auto fences = std::array {
            currentFrame.depthPrePassInFlight,
            currentFrame.computeInFlight,
            currentFrame.graphicsInFlight,
            currentFrame.postProcessInFlight,
            currentFrame.renderInFlight,
        };

        for (const auto fence : fences) {
            if (const auto result = m_device.Handle().waitForFences(1, &fence, vk::True, UINT64_MAX); result != vk::Result::eSuccess) {
                std::cerr << "Failed to wait for fence: " << vk::to_string(result) << std::endl;
                exit(1);
            }
            if (const auto result = m_device.Handle().resetFences(1, &fence); result != vk::Result::eSuccess) {
                std::cerr << "Failed to reset fence: " << vk::to_string(result) << std::endl;
            }
        }

        currentFrame.commandBuffers.at(Core::CommandBuffer::Usage::DepthPrePass).commandBuffer.reset(vk::CommandBufferResetFlagBits::eReleaseResources);
        currentFrame.commandBuffers.at(Core::CommandBuffer::Usage::Updates).commandBuffer.reset(vk::CommandBufferResetFlagBits::eReleaseResources);
        currentFrame.commandBuffers.at(Core::CommandBuffer::Usage::Graphics).commandBuffer.reset(vk::CommandBufferResetFlagBits::eReleaseResources);
        currentFrame.commandBuffers.at(Core::CommandBuffer::Usage::PostProcess).commandBuffer.reset(vk::CommandBufferResetFlagBits::eReleaseResources);
        currentFrame.commandBuffers.at(Core::CommandBuffer::Usage::Present).commandBuffer.reset(vk::CommandBufferResetFlagBits::eReleaseResources);

        if (const auto result = m_swapChain->Acquire(currentFrame);
            result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR) {
            m_swapChain->Resize(m_window.Extent());
            m_frameStarted = false;
        } else {
            currentFrame.commandBuffers.at(Core::CommandBuffer::Usage::DepthPrePass).commandBuffer.begin(vk::CommandBufferBeginInfo());
            currentFrame.commandBuffers.at(Core::CommandBuffer::Usage::Updates).commandBuffer.begin(vk::CommandBufferBeginInfo());
            currentFrame.commandBuffers.at(Core::CommandBuffer::Usage::Graphics).commandBuffer.begin(vk::CommandBufferBeginInfo());
            currentFrame.commandBuffers.at(Core::CommandBuffer::Usage::PostProcess).commandBuffer.begin(vk::CommandBufferBeginInfo());
            currentFrame.commandBuffers.at(Core::CommandBuffer::Usage::Present).commandBuffer.begin(vk::CommandBufferBeginInfo());
            m_frameStarted = true;
        }
        return m_frameStarted;
    }

    void Scheduler::Update(const float deltaTime) {

    }

    // void Scheduler::UpdateUI() {
    //     if (m_resized) {
    //         m_device.Handle().waitIdle();
    //
    //         const uint32_t imageCount = m_swapChain->ImageCount();
    //
    //         for (uint32_t i = 0; i < imageCount; i++) {
    //             ImGui_ImplVulkan_RemoveTexture(m_displayDescriptorSets[i]);
    //         }
    //
    //         m_depthPrepass->RenderPass()->Resize(imageCount, m_extent);
    //         m_reflectionPass->RenderPass()->Resize(imageCount, { m_extent.width / 2, m_extent.height / 2 });
    //         m_graphicsPass->RenderPass()->Resize(imageCount, m_extent);
    //
    //         Camera::Main()->Resize({ m_extent.width, m_extent.height });
    //
    //         for (uint32_t i = 0; i < imageCount; i++) {
    //             const auto& outputImage = m_graphicsPass->RenderPass()->OutputImage(i);
    //             m_displayDescriptorSets[i] = ImGui_ImplVulkan_AddTexture(
    //                 Memory::Sampler::Get(vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerAddressMode::eRepeat, vk::SamplerMipmapMode::eLinear),
    //                 outputImage.ImageView(),
    //                 static_cast<VkImageLayout>(vk::ImageLayout::eShaderReadOnlyOptimal));
    //         }
    //
    //         m_resized = false;
    //     }
    // }

    void Scheduler::SubmitQueue(const vk::QueueFlagBits queueType, const SubmitInfo &submitSyncInfo) const {
        const auto& currentFrame = CurrentFrame();
        const auto& commandBuffer = currentFrame.commandBuffers.at(submitSyncInfo.usage);
        commandBuffer.Handle().end();

        const auto commandBuffers = std::array { commandBuffer.Handle() };

        const auto submitInfo = vk::SubmitInfo()
            .setCommandBuffers(commandBuffers)
            .setWaitSemaphores(submitSyncInfo.waitSemaphores)
            .setSignalSemaphores(submitSyncInfo.signalSemaphores)
            .setWaitDstStageMask(submitSyncInfo.waitStages);

        try {
            m_queues.at(queueType)->Handle().submit(submitInfo, submitSyncInfo.fence);
        } catch (const vk::OutOfDateKHRError &e) {
            std::cerr << e.what() << std::endl;
        }
    }

    void Scheduler::Draw() const {
        if (!m_frameStarted) {
            std::cerr << "Called Draw() while frame is not started!" << std::endl;
        }

        const auto &presentCommandBuffer = CurrentFrame().commandBuffers.at(Core::CommandBuffer::Usage::Present);
        m_swapChain->Render(presentCommandBuffer.Handle());
    }

    // void Scheduler::DrawUI() {
    //     const uint32_t currentFrame = m_frames[m_currentFrame].imageIndex;
    //     ImGui::Begin("Main Viewport");
    //
    //     const vk::Extent2D extent = {
    //         static_cast<uint32_t>(ImGui::GetContentRegionAvail().x),
    //         static_cast<uint32_t>(ImGui::GetContentRegionAvail().y)
    //     };
    //
    //     if (extent != m_extent) {
    //         m_resized = true;
    //         m_extent = extent;
    //     }
    //
    //     const auto imageSize = ImVec2(static_cast<float>(extent.width), static_cast<float>(extent.height));
    //     ImGui::Image(m_displayDescriptorSets[currentFrame], imageSize, ImVec2(0, 1), ImVec2(1, 0));
    //
    //     ImGui::End();
    // }

    bool Scheduler::EndFrame() {
        if (!m_frameStarted) {
            std::cerr << "Called EndFrame() while frame is not started!" << std::endl;
            return false;
        }

        if (m_window.IsPaused()) {
            return false;
        }

        const auto& currentFrame = CurrentFrame();

        const auto depthPrePassSubmitInfo = SubmitInfo {
            .waitSemaphores = { currentFrame.imageAvailable },
            .waitStages = { vk::PipelineStageFlagBits::eAllCommands },
            .signalSemaphores = { currentFrame.depthPrePassFinished },
            .fence = currentFrame.depthPrePassInFlight,
            .usage = Core::CommandBuffer::Usage::DepthPrePass,
        };
        SubmitQueue(vk::QueueFlagBits::eGraphics, depthPrePassSubmitInfo);

        const auto computeSubmitInfo = SubmitInfo {
            .waitSemaphores = { CurrentFrame().depthPrePassFinished },
            .waitStages = { vk::PipelineStageFlagBits::eAllCommands },
            .signalSemaphores = { CurrentFrame().computeFinished },
            .fence = CurrentFrame().computeInFlight,
            .usage = Core::CommandBuffer::Usage::Updates,
        };
        SubmitQueue(vk::QueueFlagBits::eCompute, computeSubmitInfo);

        const auto graphicsSubmitInfo = SubmitInfo {
            .waitSemaphores = { currentFrame.computeFinished },
            .waitStages = { vk::PipelineStageFlagBits::eAllCommands },
            .signalSemaphores = { currentFrame.graphicsFinished },
            .fence = currentFrame.graphicsInFlight,
            .usage = Core::CommandBuffer::Usage::Graphics,
        };
        SubmitQueue(vk::QueueFlagBits::eGraphics, graphicsSubmitInfo);

        const auto postProcessSubmitInfo = SubmitInfo {
            .waitSemaphores = { currentFrame.graphicsFinished },
            .waitStages = { vk::PipelineStageFlagBits::eAllCommands },
            .signalSemaphores = { currentFrame.postProcessFinished },
            .fence = currentFrame.postProcessInFlight,
            .usage = Core::CommandBuffer::Usage::PostProcess,
        };
        SubmitQueue(vk::QueueFlagBits::eCompute, postProcessSubmitInfo);

        const auto presentSubmitInfo = SubmitInfo {
            .waitSemaphores = { currentFrame.postProcessFinished },
            .waitStages = { vk::PipelineStageFlagBits::eAllCommands },
            .signalSemaphores = { currentFrame.renderFinished },
            .fence = currentFrame.renderInFlight,
            .usage = Core::CommandBuffer::Usage::Present,
        };
        SubmitQueue(vk::QueueFlagBits::eGraphics, presentSubmitInfo);

        if (const auto result = m_swapChain->Present(currentFrame);
            result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR) {
            m_swapChain->Resize(m_window.Extent());
            m_frameStarted = false;
            return false;
        }

        m_frameStarted = false;
        m_currentFrame = (m_currentFrame + 1) % m_imageCount;
        return true;
    }
}
