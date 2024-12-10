//
// Created by radue on 10/14/2024.
//

#include "renderer.h"

#include <iostream>

#include "imgui.h"
#include "imgui_impl_vulkan.h"
#include "imnodes.h"
#include "components/camera.h"
#include "core/input.h"
#include "core/physicalDevice.h"
#include "core/runtime.h"
#include "core/window.h"
#include "graphics/renderPass.h"
#include "memory/buffer.h"
#include "memory/image.h"
#include "memory/sampler.h"
#include "memory/descriptor/pool.h"
#include "memory/descriptor/set.h"
#include "renderPasses/depthPrepass.h"
#include "renderPasses/graphicsPass.h"
#include "renderPasses/reflectionPass.h"

namespace mgv {
    Renderer::Renderer()
        : m_swapChainSettings {
            .minImageCount = Core::Runtime::Get().PhysicalDevice().SurfaceCapabilities().minImageCount,
            .imageCount = 2,
            .sampleCount = vk::SampleCountFlagBits::e1,
        }
    {
        CreateFrames();
        CreateDescriptorPool();

        m_swapChain = std::make_unique<Graphics::SwapChain>(
            Core::Window::Extent(),
            m_swapChainSettings);
    }

    Renderer::~Renderer() {
        const auto& device = Core::Device::Get();
        (*device).waitIdle();

        std::vector<Core::CommandBuffer> commandBuffers;

        for (auto& frame : m_frames) {
            (*device).destroySemaphore(frame.imageAvailable);
            (*device).destroyFence(frame.depthPrePassInFlight);
            (*device).destroySemaphore(frame.depthPrePassFinished);
            (*device).destroyFence(frame.computeInFlight);
            (*device).destroySemaphore(frame.computeFinished);
            (*device).destroyFence(frame.graphicsInFlight);
            (*device).destroySemaphore(frame.graphicsFinished);
            (*device).destroyFence(frame.postProcessInFlight);
            (*device).destroySemaphore(frame.postProcessFinished);
            (*device).destroyFence(frame.renderInFlight);
            (*device).destroySemaphore(frame.renderFinished);

            commandBuffers.emplace_back(frame.commandBuffers.at(Core::CommandBuffer::Usage::DepthPrePass));
            commandBuffers.emplace_back(frame.commandBuffers.at(Core::CommandBuffer::Usage::Updates));
            commandBuffers.emplace_back(frame.commandBuffers.at(Core::CommandBuffer::Usage::Graphics));
            commandBuffers.emplace_back(frame.commandBuffers.at(Core::CommandBuffer::Usage::PostProcess));
            commandBuffers.emplace_back(frame.commandBuffers.at(Core::CommandBuffer::Usage::Present));
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

    void Renderer::Init() {
        m_instance = new Renderer();

        m_instance->m_depthPrepass = std::make_unique<class DepthPrepass>();
        m_instance->m_reflectionPass = std::make_unique<class ReflectionPass>();
        m_instance->m_graphicsPass = std::make_unique<class GraphicsPass>();

        m_instance->m_cameraBuffer = std::make_unique<Memory::Buffer>(
            sizeof(mgv::Camera::Info), 1,
            vk::BufferUsageFlagBits::eUniformBuffer,
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

        m_instance->m_globalSetLayout = Memory::Descriptor::SetLayout::Builder()
            .AddBinding(0, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eAll)
            .Build();

        m_instance->m_globalDescriptorSet = Memory::Descriptor::Set::Builder(*m_instance->m_descriptorPool, *m_instance->m_globalSetLayout)
            .WriteBuffer(0, m_instance->m_cameraBuffer->DescriptorInfo())
            .Build();
    }

    void Renderer::InitUI() {
        const uint32_t imageCount = m_instance->m_swapChain->ImageCount();
        m_instance->m_displayDescriptorSets = std::vector<vk::DescriptorSet>(imageCount);
        for (uint32_t i = 0; i < imageCount; i++) {
            const auto& outputImage = m_instance->m_graphicsPass->RenderPass()->OutputImage(i);
            m_instance->m_displayDescriptorSets[i] = ImGui_ImplVulkan_AddTexture(
                Memory::Sampler::Get(vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerAddressMode::eRepeat, vk::SamplerMipmapMode::eLinear),
                outputImage.ImageView(),
                static_cast<VkImageLayout>(vk::ImageLayout::eShaderReadOnlyOptimal));
        }
    }

    void Renderer::Destroy() {
        delete m_instance;
    }

    void Renderer::DestroyUI() {
        for (uint32_t i = 0; i < m_instance->SwapChain().ImageCount(); i++) {
            ImGui_ImplVulkan_RemoveTexture(m_instance->m_displayDescriptorSets[i]);
        }
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
                .depthPrePassInFlight = (*device).createFence(fenceCreateInfo),
                .depthPrePassFinished = (*device).createSemaphore(semaphoreCreateInfo),
                .computeInFlight = (*device).createFence(fenceCreateInfo),
                .computeFinished = (*device).createSemaphore(semaphoreCreateInfo),
                .graphicsInFlight = (*device).createFence(fenceCreateInfo),
                .graphicsFinished = (*device).createSemaphore(semaphoreCreateInfo),
                .postProcessInFlight = (*device).createFence(fenceCreateInfo),
                .postProcessFinished = (*device).createSemaphore(semaphoreCreateInfo),
                .renderInFlight = (*device).createFence(fenceCreateInfo),
                .renderFinished = (*device).createSemaphore(semaphoreCreateInfo),
            });
        }

        Core::Queue* computeQueue = device.RequestQueue(vk::QueueFlagBits::eCompute).value();
        Core::Queue* graphicsQueue = device.RequestQueue(vk::QueueFlagBits::eGraphics).value();

        auto depthPrePassCommandBuffers = device.RequestCommandBuffers(graphicsQueue->familyIndex, m_swapChainSettings.imageCount);
        auto computeCommandBuffers = device.RequestCommandBuffers(computeQueue->familyIndex, m_swapChainSettings.imageCount);
        auto graphicsCommandBuffers = device.RequestCommandBuffers(graphicsQueue->familyIndex, m_swapChainSettings.imageCount);
        auto postProcessCommandBuffers = device.RequestCommandBuffers(computeQueue->familyIndex, m_swapChainSettings.imageCount);
        auto presentCommandBuffers = device.RequestCommandBuffers(graphicsQueue->familyIndex, m_swapChainSettings.imageCount);

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

        m_swapChainSettings.queueFamilyIndices = { graphicsQueue->familyIndex };
    }

    bool Renderer::BeginFrame() {
        if (m_instance->m_frameStarted) {
            std::cerr << "Called BeginFrame() while frame is already started!" << std::endl;
            return false;
        }

        if (Core::Window::IsPaused()) {
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
            const auto& device = Core::Device::Get();
            if (const auto result = (*device).waitForFences(1, &fence, vk::True, UINT64_MAX); result != vk::Result::eSuccess) {
                std::cerr << "Failed to wait for fence: " << vk::to_string(result) << std::endl;
                exit(1);
            }
            if (const auto result = (*device).resetFences(1, &fence); result != vk::Result::eSuccess) {
                std::cerr << "Failed to reset fence: " << vk::to_string(result) << std::endl;
            }
        }

        currentFrame.commandBuffers.at(Core::CommandBuffer::Usage::DepthPrePass).commandBuffer.reset(vk::CommandBufferResetFlagBits::eReleaseResources);
        currentFrame.commandBuffers.at(Core::CommandBuffer::Usage::Updates).commandBuffer.reset(vk::CommandBufferResetFlagBits::eReleaseResources);
        currentFrame.commandBuffers.at(Core::CommandBuffer::Usage::Graphics).commandBuffer.reset(vk::CommandBufferResetFlagBits::eReleaseResources);
        currentFrame.commandBuffers.at(Core::CommandBuffer::Usage::PostProcess).commandBuffer.reset(vk::CommandBufferResetFlagBits::eReleaseResources);
        currentFrame.commandBuffers.at(Core::CommandBuffer::Usage::Present).commandBuffer.reset(vk::CommandBufferResetFlagBits::eReleaseResources);

        if (const auto result = m_instance->m_swapChain->Acquire(currentFrame);
            result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR) {
            auto newSwapChain = std::make_unique<Graphics::SwapChain>(
                Core::Window::Extent(),
                m_instance->m_swapChainSettings,
                std::move(m_instance->m_swapChain));
            m_instance->m_swapChain = std::move(newSwapChain);
            m_instance->m_frameStarted = false;
        } else {
            currentFrame.commandBuffers.at(Core::CommandBuffer::Usage::DepthPrePass).commandBuffer.begin(vk::CommandBufferBeginInfo());
            currentFrame.commandBuffers.at(Core::CommandBuffer::Usage::Updates).commandBuffer.begin(vk::CommandBufferBeginInfo());
            currentFrame.commandBuffers.at(Core::CommandBuffer::Usage::Graphics).commandBuffer.begin(vk::CommandBufferBeginInfo());
            currentFrame.commandBuffers.at(Core::CommandBuffer::Usage::PostProcess).commandBuffer.begin(vk::CommandBufferBeginInfo());
            currentFrame.commandBuffers.at(Core::CommandBuffer::Usage::Present).commandBuffer.begin(vk::CommandBufferBeginInfo());
            m_instance->m_frameStarted = true;
        }
        return m_instance->m_frameStarted;
    }

    void Renderer::Update(const float deltaTime) {
        if (Camera::Main() != nullptr && Camera::Main()->Moved()) {
            auto mapped = m_instance->m_cameraBuffer->Map<Camera::Info>();
            mapped[0] = Camera::Main()->BufferData();
            m_instance->m_cameraBuffer->Flush();
            m_instance->m_cameraBuffer->Unmap();
        }

        m_instance->m_depthPrepass->RenderPass()->Update(deltaTime);
        m_instance->m_reflectionPass->RenderPass()->Update(deltaTime);
        m_instance->m_graphicsPass->RenderPass()->Update(deltaTime);
    }

    void Renderer::UpdateUI() {
        if (m_instance->m_resized) {
            (*Core::Device::Get()).waitIdle();

            const uint32_t imageCount = m_instance->m_swapChain->ImageCount();

            for (uint32_t i = 0; i < imageCount; i++) {
                ImGui_ImplVulkan_RemoveTexture(m_instance->m_displayDescriptorSets[i]);
            }

            m_instance->m_depthPrepass->RenderPass()->Resize(imageCount, m_instance->m_extent);
            m_instance->m_reflectionPass->RenderPass()->Resize(imageCount, { m_instance->m_extent.width / 2, m_instance->m_extent.height / 2 });
            m_instance->m_graphicsPass->RenderPass()->Resize(imageCount, m_instance->m_extent);

            for (uint32_t i = 0; i < imageCount; i++) {
                const auto& outputImage = m_instance->m_graphicsPass->RenderPass()->OutputImage(i);
                m_instance->m_displayDescriptorSets[i] = ImGui_ImplVulkan_AddTexture(
                    Memory::Sampler::Get(vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerAddressMode::eRepeat, vk::SamplerMipmapMode::eLinear),
                    outputImage.ImageView(),
                    static_cast<VkImageLayout>(vk::ImageLayout::eShaderReadOnlyOptimal));
            }

            m_instance->m_resized = false;
        }
    }

    void Renderer::SubmitQueue(const vk::QueueFlagBits queueType, const SubmitInfo &submitSyncInfo) const {
        const auto& currentFrame = CurrentFrame();
        const auto& commandBuffer = currentFrame.commandBuffers.at(submitSyncInfo.usage);
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

    void Renderer::Draw() {
        if (!m_instance->m_frameStarted) {
            std::cerr << "Called Draw() while frame is not started!" << std::endl;
        }

        const auto &depthPrePassCommandBuffer = CurrentFrame().commandBuffers.at(Core::CommandBuffer::Usage::DepthPrePass);
        m_instance->m_depthPrepass->Run(*depthPrePassCommandBuffer);

        const auto &graphicsCommandBuffer = CurrentFrame().commandBuffers.at(Core::CommandBuffer::Usage::Graphics);
        m_instance->m_reflectionPass->Run(*graphicsCommandBuffer);
        m_instance->m_graphicsPass->Run(*graphicsCommandBuffer);

        const auto &presentCommandBuffer = CurrentFrame().commandBuffers.at(Core::CommandBuffer::Usage::Present);
        m_instance->m_swapChain->Render(*presentCommandBuffer);
    }

    void Renderer::DrawUI() {
        uint32_t currentFrame = m_instance->m_frames[m_instance->m_currentFrame].imageIndex;
        ImGui::Begin("Main Viewport");

        const vk::Extent2D extent = {
            static_cast<uint32_t>(ImGui::GetContentRegionAvail().x),
            static_cast<uint32_t>(ImGui::GetContentRegionAvail().y)
        };

        if (extent != m_instance->m_extent) {
            m_instance->m_resized = true;
            m_instance->m_extent = extent;
        }

        const auto imageSize = ImVec2(static_cast<float>(extent.width), static_cast<float>(extent.height));
        ImGui::Image(m_instance->m_displayDescriptorSets[currentFrame], imageSize, ImVec2(0, 1), ImVec2(1, 0));

        ImGui::End();
    }

    bool Renderer::EndFrame() {
        if (!m_instance->m_frameStarted) {
            std::cerr << "Called EndFrame() while frame is not started!" << std::endl;
            return false;
        }

        if (Core::Window::IsPaused()) {
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
        m_instance->SubmitQueue(vk::QueueFlagBits::eGraphics, depthPrePassSubmitInfo);

        const auto computeSubmitInfo = SubmitInfo {
            .waitSemaphores = { CurrentFrame().depthPrePassFinished },
            .waitStages = { vk::PipelineStageFlagBits::eAllCommands },
            .signalSemaphores = { CurrentFrame().computeFinished },
            .fence = CurrentFrame().computeInFlight,
            .usage = Core::CommandBuffer::Usage::Updates,
        };
        m_instance->SubmitQueue(vk::QueueFlagBits::eCompute, computeSubmitInfo);

        const auto graphicsSubmitInfo = SubmitInfo {
            .waitSemaphores = { currentFrame.computeFinished },
            .waitStages = { vk::PipelineStageFlagBits::eAllCommands },
            .signalSemaphores = { currentFrame.graphicsFinished },
            .fence = currentFrame.graphicsInFlight,
            .usage = Core::CommandBuffer::Usage::Graphics,
        };
        m_instance->SubmitQueue(vk::QueueFlagBits::eGraphics, graphicsSubmitInfo);

        const auto postProcessSubmitInfo = SubmitInfo {
            .waitSemaphores = { currentFrame.graphicsFinished },
            .waitStages = { vk::PipelineStageFlagBits::eAllCommands },
            .signalSemaphores = { currentFrame.postProcessFinished },
            .fence = currentFrame.postProcessInFlight,
            .usage = Core::CommandBuffer::Usage::PostProcess,
        };
        m_instance->SubmitQueue(vk::QueueFlagBits::eCompute, postProcessSubmitInfo);

        const auto presentSubmitInfo = SubmitInfo {
            .waitSemaphores = { currentFrame.postProcessFinished },
            .waitStages = { vk::PipelineStageFlagBits::eAllCommands },
            .signalSemaphores = { currentFrame.renderFinished },
            .fence = currentFrame.renderInFlight,
            .usage = Core::CommandBuffer::Usage::Present,
        };
        m_instance->SubmitQueue(vk::QueueFlagBits::eGraphics, presentSubmitInfo);

        if (const auto result = m_instance->m_swapChain->Present(currentFrame);
            result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR) {
            m_instance->m_swapChain = std::make_unique<Graphics::SwapChain>(
                Core::Window::Extent(),
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
