//
// Created by radue on 10/14/2024.
//
#pragma once

#include <vulkan/vulkan.hpp>

#include "../core/window.h"
#include "../core/device.h"
#include "memory/descriptor/pool.h"
#include "swapChain.h"
#include "gui/manager.h"
#include "renderPasses/mainViewport.h"

namespace GUI {
    class Manager;
}

namespace mgv {
    struct Frame {
        uint32_t imageIndex;

        vk::Semaphore imageAvailable;
        vk::Fence computeInFlight;
        vk::Semaphore computeFinished;
        vk::Fence graphicsInFlight;
        vk::Semaphore renderFinished;

        std::unordered_map<vk::QueueFlagBits, Core::CommandBuffer> commandBuffers;
    };

    struct SubmitInfo {
        std::vector<vk::Semaphore> waitSemaphores;
        std::vector<vk::PipelineStageFlags> waitStages;
        std::vector<vk::Semaphore> signalSemaphores;
        vk::Fence fence = nullptr;
    };

    class Renderer {
        friend class GUI::Manager;
    public:
        static void Init(const Core::Window &window, Core::Device &device);
        static void Destroy();

        Renderer(const Renderer &) = delete;
        Renderer &operator=(const Renderer &) = delete;

        static const Renderer* Instance() { return m_instance; }

        static bool BeginFrame();
        static void Draw();
        static bool EndFrame();

        [[nodiscard]] static const Graphics::SwapChain &SwapChain() { return *m_instance->m_swapChain; }
        [[nodiscard]] static const Memory::Descriptor::Pool &DescriptorPool() { return *m_instance->m_descriptorPool; }
        [[nodiscard]] static const Frame &CurrentFrame() { return m_instance->m_frames[m_instance->m_currentFrame]; }
        [[nodiscard]] static bool IsFrameStarted() { return m_instance->m_frameStarted; }

        // TODO: Find a better way to handle this
        static void SetMainViewport(MainViewport *mainViewport) { m_instance->m_mainViewport = mainViewport; }

    private:
        Renderer(const Core::Window &window, Core::Device &device);
        ~Renderer();

        void SubmitQueue(vk::QueueFlagBits queueType, const SubmitInfo &submitSyncInfo) const;
        void CreateFrames();

        static Renderer* m_instance;

        const Core::Window &m_window;
        Core::Device &m_device;

        bool m_frameStarted = false;
        uint32_t m_currentFrame = 0;

        std::unordered_map<vk::QueueFlagBits, Core::Queue*> m_queues;
        std::vector<Frame> m_frames;

        MainViewport *m_mainViewport = nullptr;

        Graphics::SwapChain::Settings m_swapChainSettings;
        std::unique_ptr<Graphics::SwapChain> m_swapChain;

        void CreateDescriptorPool();
        std::unique_ptr<Memory::Descriptor::Pool> m_descriptorPool;
    };
}
