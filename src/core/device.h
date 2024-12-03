//
// Created by radue on 10/14/2024.
//

#pragma once

#include <functional>
#include <optional>
#include <unordered_map>

#include "physicalDevice.h"
#include "runtime.h"

namespace Core {
    class Device;

    struct Queue {
        uint32_t familyIndex;
        uint32_t index;
        bool inUse = false;
        vk::Queue queue;

        vk::Queue operator *() const { return queue; }

        Queue(const Device& device, uint32_t familyIndex, uint32_t index);
    };

    struct CommandBuffer {
        uint32_t familyIndex;
        vk::CommandBuffer commandBuffer;

        vk::CommandBuffer operator *() const { return commandBuffer; }
    };

    struct QueueFamily
    {
        uint32_t index;
        vk::QueueFamilyProperties properties;
        bool presentSupport;
        std::vector<std::unique_ptr<Queue>> queues;
    };

    class Device {
    public:
        static void Init();
        static void Destroy();

        static Device& Get() { return *m_instance; }

        Device(const Device &) = delete;
        Device &operator=(const Device &) = delete;

        vk::Device operator *() const { return m_device; }
        [[nodiscard]] std::optional<Queue*> RequestQueue(vk::QueueFlags type) const;
        [[nodiscard]] std::optional<Queue*> RequestPresentQueue() const;

        [[nodiscard]] uint32_t GetQueueFamilyIndex(vk::QueueFlags type) const;
        [[nodiscard]] std::vector<CommandBuffer> RequestCommandBuffers(uint32_t familyIndex, uint32_t count, uint32_t thread = 0) const;
        void FreeCommandBuffers(const std::vector<CommandBuffer> &commandBuffers, uint32_t thread = 0) const;

        void QuerySurfaceCapabilities() const;
        [[nodiscard]] std::optional<uint32_t> FindMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) const;

        void RunSingleTimeCommand(const std::function<void(vk::CommandBuffer)> &command, vk::QueueFlags requiredFlags, vk::Fence fence = nullptr, uint32_t thread = 0) const;

        explicit Device();
        ~Device();
    private:
        static std::unique_ptr<Device> m_instance;

        vk::Device m_device;
        std::vector<QueueFamily> m_queueFamilies;
        std::unordered_map<uint32_t, std::vector<vk::CommandPool>> m_commandPools;
    };
}