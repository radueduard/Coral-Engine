//
// Created by radue on 10/14/2024.
//

#include "renderer.h"

#include <iostream>
#include "components/camera.h"

namespace mgv {
    Renderer::Renderer()
        : m_swapChainSettings {
            .minImageCount = Core::Runtime::Get().PhysicalDevice().SurfaceCapabilities().minImageCount,
            .imageCount = 2,
            .sampleCount = vk::SampleCountFlagBits::e2,
        }
    {
        CreateFrames();
        CreateDescriptorPool();

        m_swapChain = std::make_unique<Graphics::SwapChain>(
            Core::Window::Get().Extent(),
            m_swapChainSettings);
    }

    Renderer::~Renderer() {
        const auto& device = Core::Device::Get();
        (*device).waitIdle();

        std::vector<Core::CommandBuffer> commandBuffers;

        for (auto& frame : m_frames) {
            (*device).destroySemaphore(frame.imageAvailable);
            (*device).destroyFence(frame.computeInFlight);
            (*device).destroySemaphore(frame.computeFinished);
            (*device).destroyFence(frame.graphicsInFlight);
            (*device).destroySemaphore(frame.renderFinished);

            commandBuffers.emplace_back(frame.commandBuffers.at(vk::QueueFlagBits::eGraphics));
            commandBuffers.emplace_back(frame.commandBuffers.at(vk::QueueFlagBits::eCompute));
        }

        device.FreeCommandBuffers(commandBuffers);
    }

    void Renderer::CreateDescriptorPool() {
        m_descriptorPool = Memory::Descriptor::Pool::Builder()
            .AddPoolSize(vk::DescriptorType::eUniformBuffer, 100)
            .AddPoolSize(vk::DescriptorType::eStorageBuffer, 100)
            .AddPoolSize(vk::DescriptorType::eCombinedImageSampler, 100)
            .PoolFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet)
            .MaxSets(100)
            .Build();
    }

    std::unique_ptr<Renderer> Renderer::m_instance = nullptr;

    void Renderer::Init() {
        m_instance = std::make_unique<Renderer>();
    }

    void Renderer::Destroy() {
        m_instance.reset();
    }

    void Renderer::CreateFrames() {
        const auto& device = Core::Device::Get();
        m_frames.reserve(m_swapChainSettings.imageCount);
        for (uint32_t i = 0; i < m_swapChainSettings.imageCount; i++) {
            constexpr auto fenceCreateInfo = vk::FenceCreateInfo()
                .setFlags(vk::FenceCreateFlagBits::eSignaled);
            constexpr auto semaphoreCreateInfo = vk::SemaphoreCreateInfo();

            m_frames.emplace_back(Frame {
                .imageIndex = i,
                .imageAvailable = (*device).createSemaphore(semaphoreCreateInfo),
                .computeInFlight = (*device).createFence(fenceCreateInfo),
                .computeFinished = (*device).createSemaphore(semaphoreCreateInfo),
                .graphicsInFlight = (*device).createFence(fenceCreateInfo),
                .renderFinished = (*device).createSemaphore(semaphoreCreateInfo),
            });
        }

        Core::Queue* computeQueue = device.RequestQueue(vk::QueueFlagBits::eCompute).value();
        Core::Queue* graphicsQueue = device.RequestQueue(vk::QueueFlagBits::eGraphics).value();

        auto computeCommandBuffers = device.RequestCommandBuffers(computeQueue->familyIndex, m_swapChainSettings.imageCount);
        auto graphicsCommandBuffers = device.RequestCommandBuffers(graphicsQueue->familyIndex, m_swapChainSettings.imageCount);

        m_queues = {
            { vk::QueueFlagBits::eCompute, computeQueue },
            { vk::QueueFlagBits::eGraphics, graphicsQueue }
        };

        for (auto& frame : m_frames) {
            frame.commandBuffers = {
                { vk::QueueFlagBits::eCompute, computeCommandBuffers.at(frame.imageIndex) },
                { vk::QueueFlagBits::eGraphics, graphicsCommandBuffers.at(frame.imageIndex) }
            };
        }

        m_swapChainSettings.queueFamilyIndices = { graphicsQueue->familyIndex };
    }

    bool Renderer::BeginFrame() {
        if (m_instance->m_frameStarted) {
            std::cerr << "Called BeginFrame() while frame is already started!" << std::endl;
            return false;
        }

        if (Core::Window::Get().IsPaused()) {
            return false;
        }

        const auto currentFrame = CurrentFrame();

        const auto fences = std::array {
            currentFrame.computeInFlight,
            currentFrame.graphicsInFlight
        };

        for (const auto fence : fences) {
            const auto& device = Core::Device::Get();
            if (const auto result = (*device).waitForFences(1, &fence, vk::True, UINT64_MAX); result != vk::Result::eSuccess) {
                std::cerr << "Failed to wait for fence: " << vk::to_string(result) << std::endl;
                exit(1);
            }
            if (const auto result = (*device).resetFences(1, &fence); result != vk::Result::eSuccess) {
                std::cerr << "Failed to reset fence: " << vk::to_string(result) << std::endl;
            }
        }

        currentFrame.commandBuffers.at(vk::QueueFlagBits::eCompute).commandBuffer.reset(vk::CommandBufferResetFlagBits::eReleaseResources);
        currentFrame.commandBuffers.at(vk::QueueFlagBits::eGraphics).commandBuffer.reset(vk::CommandBufferResetFlagBits::eReleaseResources);

        if (const auto result = m_instance->m_swapChain->Acquire(currentFrame);
            result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR) {
            auto newSwapChain = std::make_unique<Graphics::SwapChain>(
                Core::Window::Get().Extent(),
                m_instance->m_swapChainSettings,
                std::move(m_instance->m_swapChain));
            m_instance->m_swapChain = std::move(newSwapChain);
            m_instance->m_frameStarted = false;
        } else {
            currentFrame.commandBuffers.at(vk::QueueFlagBits::eCompute).commandBuffer.begin(vk::CommandBufferBeginInfo());
            currentFrame.commandBuffers.at(vk::QueueFlagBits::eGraphics).commandBuffer.begin(vk::CommandBufferBeginInfo());
            m_instance->m_frameStarted = true;
        }
        return m_instance->m_frameStarted;
    }

    void Renderer::Update(const float deltaTime) {
        m_instance->m_reflectionPass->RenderPass().Update(deltaTime);
        m_instance->m_mainViewport->RenderPass().Update(deltaTime);
        Compute();
    }

    void Renderer::SubmitQueue(const vk::QueueFlagBits queueType, const SubmitInfo &submitSyncInfo) const {
        const auto& currentFrame = CurrentFrame();
        const auto& commandBuffer = currentFrame.commandBuffers.at(queueType);
        (*commandBuffer).end();

        const auto commandBuffers = std::array { *commandBuffer };

        const auto submitInfo = vk::SubmitInfo()
            .setCommandBuffers(commandBuffers)
            .setWaitSemaphores(submitSyncInfo.waitSemaphores)
            .setSignalSemaphores(submitSyncInfo.signalSemaphores)
            .setWaitDstStageMask(submitSyncInfo.waitStages);

        try {
            (**m_queues.at(queueType)).submit(submitInfo, submitSyncInfo.fence);
        } catch (const vk::OutOfDateKHRError &e) {
            std::cerr << e.what() << std::endl;
        }
    }

    void Renderer::Compute() {
        if (!m_instance->m_frameStarted) {
            std::cerr << "Called Draw() while frame is not started!" << std::endl;
        }

        const auto& commandBuffer = CurrentFrame().commandBuffers.at(vk::QueueFlagBits::eCompute);
        for (const auto& program : m_instance->m_computePrograms) {
            program->Compute(*commandBuffer);
        }
    }

    void Renderer::Draw() {
        if (!m_instance->m_frameStarted) {
            std::cerr << "Called Draw() while frame is not started!" << std::endl;
        }

        const auto &commandBuffer = CurrentFrame().commandBuffers.at(vk::QueueFlagBits::eGraphics);
        m_instance->m_reflectionPass->Run(*commandBuffer);
        m_instance->m_mainViewport->Run(*commandBuffer);
        m_instance->m_swapChain->Render(*commandBuffer);
    }

    bool Renderer::EndFrame() {
        if (!m_instance->m_frameStarted) {
            std::cerr << "Called EndFrame() while frame is not started!" << std::endl;
            return false;
        }

        if (Core::Window::Get().IsPaused()) {
            return false;
        }

        const auto& currentFrame = CurrentFrame();

        const auto computeSubmitInfo = SubmitInfo {
            .waitSemaphores = { CurrentFrame().imageAvailable },
            .waitStages = { vk::PipelineStageFlagBits::eAllCommands },
            .signalSemaphores = { CurrentFrame().computeFinished },
            .fence = CurrentFrame().computeInFlight,
        };
        m_instance->SubmitQueue(vk::QueueFlagBits::eCompute, computeSubmitInfo);

        const auto graphicsSubmitInfo = SubmitInfo {
            .waitSemaphores = { currentFrame.computeFinished },
            .waitStages = { vk::PipelineStageFlagBits::eAllCommands },
            .signalSemaphores = { currentFrame.renderFinished },
            .fence = currentFrame.graphicsInFlight,
        };
        m_instance->SubmitQueue(vk::QueueFlagBits::eGraphics, graphicsSubmitInfo);

        if (const auto result = m_instance->m_swapChain->Present(currentFrame);
            result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR) {
            m_instance->m_swapChain = std::make_unique<Graphics::SwapChain>(
                Core::Window::Get().Extent(),
                m_instance->m_swapChainSettings,
                std::move(m_instance->m_swapChain));
            m_instance->m_frameStarted = false;
            return false;
        }
        m_instance->m_frameStarted = false;
        m_instance->m_currentFrame = (m_instance->m_currentFrame + 1) % m_instance->m_swapChainSettings.imageCount;
        return true;
    }
}
