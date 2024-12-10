//
// Created by radue on 10/14/2024.
//
#pragma once

#include <unordered_map>
#include <vulkan/vulkan.hpp>

#include "core/device.h"
#include "graphics/swapChain.h"

#include "memory/buffer.h"

namespace GUI {
    class Manager;
}

namespace Graphics {
    class RenderPass;
}

namespace Memory::Descriptor {
    class Pool;
    class SetLayout;
    class Set;
}

namespace mgv {
    class Camera;
}

class DepthPrepass;
class ReflectionPass;
class GraphicsPass;

namespace mgv {
    struct Frame {
        uint32_t imageIndex;

        vk::Semaphore imageAvailable;
        vk::Fence depthPrePassInFlight;
        vk::Semaphore depthPrePassFinished;
        vk::Fence computeInFlight;
        vk::Semaphore computeFinished;
        vk::Fence graphicsInFlight;
        vk::Semaphore graphicsFinished;
        vk::Fence postProcessInFlight;
        vk::Semaphore postProcessFinished;
        vk::Fence renderInFlight;
        vk::Semaphore renderFinished;

        std::unordered_map<Core::CommandBuffer::Usage, Core::CommandBuffer> commandBuffers;
    };

    struct SubmitInfo {
        std::vector<vk::Semaphore> waitSemaphores;
        std::vector<vk::PipelineStageFlags> waitStages;
        std::vector<vk::Semaphore> signalSemaphores;
        vk::Fence fence = nullptr;
        Core::CommandBuffer::Usage usage;
    };

    class Renderer {
        friend class GUI::Manager;
    public:
        static void Init();
        static void Destroy();

        Renderer(const Renderer &) = delete;
        Renderer &operator=(const Renderer &) = delete;

        static bool BeginFrame();
        static void Update(float deltaTime);
        static void Draw();
        static bool EndFrame();

        [[nodiscard]] static const Graphics::SwapChain &SwapChain() { return *m_instance->m_swapChain; }
        [[nodiscard]] static const Memory::Descriptor::Pool &DescriptorPool() { return *m_instance->m_descriptorPool; }
        [[nodiscard]] static const Frame &CurrentFrame() { return m_instance->m_frames[m_instance->m_currentFrame]; }
        [[nodiscard]] static const vk::Extent2D &Extent() { return m_instance->m_extent; }
        [[nodiscard]] static bool IsResized() { return m_instance->m_resized; }
        [[nodiscard]] static uint32_t ImageCount() { return m_instance->m_swapChainSettings.imageCount; }
        [[nodiscard]] static bool IsFrameStarted() { return m_instance->m_frameStarted; }

        static const Core::Queue& Queue(const vk::QueueFlagBits queueType) { return *m_instance->m_queues.at(queueType); }

        static const DepthPrepass& DepthPrepass() { return *m_instance->m_depthPrepass; }
        static const ReflectionPass& ReflectionPass() { return *m_instance->m_reflectionPass; }
        static const GraphicsPass& GraphicsPass() { return *m_instance->m_graphicsPass; }

        static Memory::Descriptor::SetLayout& GlobalSetLayout() { return *m_instance->m_globalSetLayout; }
        static Memory::Descriptor::Set& GlobalDescriptorSet() { return *m_instance->m_globalDescriptorSet; }

        static void InitUI();
        static void UpdateUI();
        static void DrawUI();
        static void DestroyUI();
    private:
        Renderer();
        ~Renderer();

        void SubmitQueue(vk::QueueFlagBits queueType, const SubmitInfo &submitSyncInfo) const;
        void CreateFrames();

        inline static Renderer* m_instance;

        bool m_frameStarted = false;
        uint32_t m_currentFrame = 0;

        std::unordered_map<vk::QueueFlagBits, Core::Queue*> m_queues;
        std::vector<Frame> m_frames;

        std::unique_ptr<class DepthPrepass> m_depthPrepass;
        std::unique_ptr<class ReflectionPass> m_reflectionPass;
        std::unique_ptr<class GraphicsPass> m_graphicsPass;

        Graphics::SwapChain::Settings m_swapChainSettings;
        std::unique_ptr<Graphics::SwapChain> m_swapChain;

        void CreateDescriptorPool();
        std::unique_ptr<Memory::Descriptor::Pool> m_descriptorPool;

        std::unique_ptr<Memory::Buffer> m_cameraBuffer;
        std::unique_ptr<Memory::Descriptor::SetLayout> m_globalSetLayout;
        std::unique_ptr<Memory::Descriptor::Set> m_globalDescriptorSet;

        bool m_resized = false;
        vk::Extent2D m_extent;
        std::vector<vk::DescriptorSet> m_displayDescriptorSets;
    };
}
