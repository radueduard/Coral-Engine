//
// Created by radue on 10/14/2024.
//
#pragma once

#include <vector>
#include <ranges>

#include <vulkan/vulkan.hpp>

#include "device.h"
#include "graphics/swapChain.h"
#include "gui/container.h"
#include "project/renderGraph.h"

class Engine;

namespace Coral::Core {
    class Window;
}

namespace Coral::Reef {
    class Manager;
}

namespace Coral::Graphics {
    class RenderPass;
}

namespace Coral::Memory::Descriptor {
    class Pool;
    class SetLayout;
    class Set;
}

namespace Coral::Core {
    class Frame {
    public:
        explicit Frame(uint32_t imageIndex);
        ~Frame();

        Frame(const Frame&) = delete;
        Frame& operator=(const Frame&) = delete;

        [[nodiscard]] uint32_t ImageIndex() const { return m_imageIndex; }
        [[nodiscard]] vk::Semaphore ImageAvailable() const { return m_imageAvailable; }
        [[nodiscard]] vk::Semaphore ReadyToPresent() const { return m_readyToPresent; }
        [[nodiscard]] vk::Fence InFlightFence() const { return m_inFlightFence; }

    private:
        uint32_t m_imageIndex;
        vk::Semaphore m_imageAvailable;
        vk::Semaphore m_readyToPresent;
        vk::Fence m_inFlightFence;
    };

    struct SubmitInfo {
        const CommandBuffer& commandBuffer;
        std::vector<vk::Semaphore> waitSemaphores;
        std::vector<vk::PipelineStageFlags> waitStages;
        std::vector<vk::Semaphore> signalSemaphores;
        vk::Fence fence = nullptr;
    };

    class Scheduler {
        friend class Reef::Manager;
    public:
        struct CreateInfo {
            uint32_t minImageCount;
            uint32_t imageCount;
            vk::SampleCountFlagBits multiSampling;
            bool enableGUI = true;
        };

        explicit Scheduler(const CreateInfo &createInfo);
        ~Scheduler();
        Scheduler(const Scheduler &) = delete;
        Scheduler &operator=(const Scheduler &) = delete;

        void Update(float deltaTime);
        void Draw();

        [[nodiscard]] const Graphics::SwapChain &SwapChain() const { return *m_swapChain; }
        [[nodiscard]] const Memory::Descriptor::Pool &DescriptorPool() const { return *m_descriptorPool; }
        [[nodiscard]] const Frame &CurrentFrame() const { return *m_frames.at(m_swapChain->CurrentImageIndex()); }
        [[nodiscard]] const vk::Extent2D &Extent() const { return m_extent; }
        [[nodiscard]] bool IsResized() const { return m_resized; }
        [[nodiscard]] uint32_t ImageCount() const { return m_imageCount; }
        [[nodiscard]] auto Frames() { return m_frames | std::views::transform([](const auto& frame) { return frame.get(); }) | std::ranges::to<std::vector<Frame*>>(); }

        [[nodiscard]] Project::RenderGraph& RenderGraph() const { return *m_renderGraph; }
        Project::RenderGraph& RenderGraph() { return *m_renderGraph; }

    private:
        std::unique_ptr<Reef::Manager> m_guiManager;
        Reef::Container<Project::RenderGraph> m_renderGraph = nullptr;
        std::unique_ptr<Graphics::SwapChain> m_swapChain;


        void CreateFrames();
        unsigned int m_imageCount;
        std::vector<std::unique_ptr<Frame>> m_frames;


        void CreateDescriptorPool();
        std::unique_ptr<Memory::Descriptor::Pool> m_descriptorPool;

        bool m_resized = false;
        vk::Extent2D m_extent;
    };

    inline Scheduler* g_scheduler = nullptr;
    static Scheduler &GlobalScheduler() {
        return *g_scheduler;
    }
}
