//
// Created by radue on 10/14/2024.
//

#pragma once

#include <functional>
#include <optional>
#include <unordered_map>

#include <vulkan/vulkan.hpp>

#include "core/runtime.h"
#include "utils/globalWrapper.h"

namespace Coral::Core {
    class PhysicalDevice;
}

namespace Coral::Core {
    class Queue final : public EngineWrapper<vk::Queue> {
    public:
        class Family
        {
            friend class Queue;
        public:
            explicit Family(uint32_t index, const vk::QueueFamilyProperties &properties, bool canPresent);
            ~Family() = default;

            [[nodiscard]] uint32_t Index() const { return m_index; }
            [[nodiscard]] const vk::QueueFamilyProperties& Properties() const { return m_properties; }
            [[nodiscard]] bool CanPresent() const { return m_canPresent; }

            [[nodiscard]] std::unique_ptr<Queue> RequestQueue();
            [[nodiscard]] std::unique_ptr<Queue> RequestPresentQueue();

        private:
            uint32_t m_index;
            vk::QueueFamilyProperties m_properties;
            uint32_t m_remainingQueues = 0;
            bool m_canPresent = false;
        };

        explicit Queue(Family& family);
        ~Queue() override;

        [[nodiscard]] uint32_t Index() const { return m_index; }
        [[nodiscard]] const Family& Family() const { return m_family; }

    private:
        uint32_t m_index;
        class Family& m_family;
    };

    class CommandBuffer final : public EngineWrapper<vk::CommandBuffer> {
    public:
        CommandBuffer(const Core::Queue& queue, vk::CommandBuffer commandBuffer, const vk::CommandPool& parentCommandPool);
        ~CommandBuffer() override;
		void Run(const std::function<void(const Core::CommandBuffer&)>& command, vk::Semaphore waitSemaphore = nullptr) const;

		[[nodiscard]] const vk::CommandPool& ParentPool() const { return m_parentPool; }
        [[nodiscard]] const vk::Semaphore& SignalSemaphore() const { return m_signalSemaphore; }
        [[nodiscard]] const vk::Fence& Fence() const { return m_fence; }
    	[[nodiscard]] const Core::Queue& Queue() const { return m_queue; }

    private:
    	const Core::Queue& m_queue;
        const vk::CommandPool& m_parentPool;

        vk::Semaphore m_signalSemaphore;
        vk::Fence m_fence;
    };

    class Device final : public EngineWrapper<vk::Device> {
    public:
        explicit Device();
        ~Device() override;

        Device(const Device &) = delete;
        Device &operator=(const Device &) = delete;

        [[nodiscard]] std::unique_ptr<Queue> RequestQueue(vk::QueueFlags type);
        [[nodiscard]] std::unique_ptr<Queue> RequestPresentQueue();

        void CreateCommandPools(uint32_t threadId);
        void FreeCommandPools(uint32_t threadId);

        [[nodiscard]] std::unique_ptr<CommandBuffer> RequestCommandBuffer(const Core::Queue& queue, uint32_t thread = 0) const;
        void FreeCommandBuffer(const CommandBuffer &commandBuffer) const;

        [[nodiscard]] const PhysicalDevice& QuerySurfaceCapabilities() const;
        [[nodiscard]] std::optional<uint32_t> FindMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) const;

        void RunSingleTimeCommand(const std::function<void(const Core::CommandBuffer&)> &command, vk::QueueFlags requiredFlags,
            vk::Fence fence = nullptr, vk::Semaphore waitSemaphore = nullptr, vk::Semaphore signalSemaphore = nullptr, bool wait = true);

    private:
        std::vector<class Queue::Family> m_queueFamilies;
        std::unordered_map<uint32_t, std::unordered_map<uint32_t, vk::CommandPool>> m_commandPools;
    };
}
