//
// Created by radue on 10/14/2024.
//

#pragma once

#include <unordered_map>
#include <vulkan/vulkan.hpp>

#include "device.h"
#include "../graphics/swapChain.h"

#include "../memory/buffer.h"

namespace Core {
    class Window;
}

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

namespace Core {
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

        std::unordered_map<CommandBuffer::Usage, CommandBuffer> commandBuffers;
    };

    struct SubmitInfo {
        std::vector<vk::Semaphore> waitSemaphores;
        std::vector<vk::PipelineStageFlags> waitStages;
        std::vector<vk::Semaphore> signalSemaphores;
        vk::Fence fence = nullptr;
        CommandBuffer::Usage usage;
    };

    class Scheduler {
        friend class GUI::Manager;
    public:
        struct CreateInfo {
            const Window &window;
            const Runtime &runtime;
            const Device &device;

            uint32_t minImageCount;
            uint32_t imageCount;
            vk::SampleCountFlagBits multiSampling;
        };

        explicit Scheduler(const CreateInfo &createInfo);
        ~Scheduler();
        Scheduler(const Scheduler &) = delete;
        Scheduler &operator=(const Scheduler &) = delete;

        bool BeginFrame();
        void Update(float deltaTime);
        void Draw() const;
        bool EndFrame();

        [[nodiscard]] const Graphics::SwapChain &SwapChain() const { return *m_swapChain; }
        [[nodiscard]] const Memory::Descriptor::Pool &DescriptorPool() const { return *m_descriptorPool; }
        [[nodiscard]] const Frame &CurrentFrame() const { return m_frames.at(m_currentFrame); }
        [[nodiscard]] const vk::Extent2D &Extent() const { return m_extent; }
        [[nodiscard]] bool IsResized() const { return m_resized; }
        [[nodiscard]] uint32_t ImageCount() const { return m_imageCount; }
        [[nodiscard]] bool IsFrameStarted() const { return m_frameStarted; }
        [[nodiscard]] const Queue& Queue(const vk::QueueFlagBits queueType) const { return *m_queues.at(queueType); }

    private:
        const Window& m_window;
        const Runtime& m_runtime;
        const Device& m_device;

        void SubmitQueue(vk::QueueFlagBits queueType, const SubmitInfo &submitSyncInfo) const;
        void CreateFrames();

        unsigned int m_imageCount;
        bool m_frameStarted = false;
        uint32_t m_currentFrame = 0;

        std::unordered_map<vk::QueueFlagBits, Core::Queue*> m_queues;
        std::vector<Frame> m_frames;

        std::unique_ptr<Graphics::SwapChain> m_swapChain;

        void CreateDescriptorPool();
        std::unique_ptr<Memory::Descriptor::Pool> m_descriptorPool;

        bool m_resized = false;
        vk::Extent2D m_extent;
    };
}
