//
// Created by radue on 10/14/2024.
//
#pragma once

#include <vulkan/vulkan.hpp>

#include "core/window.h"
#include "core/device.h"
#include "memory/descriptor/pool.h"
#include "swapChain.h"
#include "compute/programs/program.h"
#include "gui/manager.h"
#include "renderPasses/mainViewport.h"
#include "renderPasses/reflectionPass.h"

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
        static void Init();
        static void Destroy();

        Renderer(const Renderer &) = delete;
        Renderer &operator=(const Renderer &) = delete;

        static Renderer& Get() { return *m_instance; }

        static void AddComputeProgram(Compute::Program *program) {
            m_instance->m_computePrograms.emplace_back(program);
        }
        static void RemoveComputeProgram(Compute::Program *program) {
            std::erase(m_instance->m_computePrograms, program);
        }

        static bool BeginFrame();
        static void Update(float deltaTime);
        static void Compute();
        static void Draw();
        static bool EndFrame();

        [[nodiscard]] static const Graphics::SwapChain &SwapChain() { return *m_instance->m_swapChain; }
        [[nodiscard]] static const Memory::Descriptor::Pool &DescriptorPool() { return *m_instance->m_descriptorPool; }
        [[nodiscard]] static const Frame &CurrentFrame() { return m_instance->m_frames[m_instance->m_currentFrame]; }
        [[nodiscard]] static bool IsFrameStarted() { return m_instance->m_frameStarted; }

        // TODO: Find a better way to handle this
        static void SetMainViewport(MainViewport *mainViewport) { m_instance->m_mainViewport = mainViewport; }
        static void SetReflectionPass(ReflectionPass *reflectionPass) { m_instance->m_reflectionPass = reflectionPass; }

        explicit Renderer();
        ~Renderer();
    private:

        void SubmitQueue(vk::QueueFlagBits queueType, const SubmitInfo &submitSyncInfo) const;
        void CreateFrames();

        static std::unique_ptr<Renderer> m_instance;

        bool m_frameStarted = false;
        uint32_t m_currentFrame = 0;

        std::unordered_map<vk::QueueFlagBits, Core::Queue*> m_queues;
        std::vector<Frame> m_frames;

        std::vector<Compute::Program*> m_computePrograms;

        MainViewport *m_mainViewport = nullptr;
        ReflectionPass *m_reflectionPass = nullptr;

        Graphics::SwapChain::Settings m_swapChainSettings;
        std::unique_ptr<Graphics::SwapChain> m_swapChain;

        void CreateDescriptorPool();
        std::unique_ptr<Memory::Descriptor::Pool> m_descriptorPool;
    };
}
