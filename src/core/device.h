//
// Created by radue on 10/14/2024.
//

#pragma once

#include <optional>
#include <unordered_map>

#include "physicalDevice.h"

namespace Core {

    struct QueueFamily
    {
        uint32_t index;
        vk::QueueFamilyProperties properties;
        bool presentSupport;
        uint32_t remainingQueues;
    };

    struct Queue
    {
        enum class Type
        {
            Graphics,
            Compute,
            Transfer,
            Present,
            Unassigned
        };

        uint32_t familyIndex;
        uint32_t index;
        vk::Queue queue = nullptr;
        Type type = Type::Unassigned;

        static Type TypeFromFlag(vk::QueueFlagBits);
    };

    class Device {
    public:
        explicit Device(PhysicalDevice& physicalDevice);
        ~Device();

        Device(const Device &) = delete;
        Device &operator=(const Device &) = delete;

        vk::Device operator *() const { return m_device; }
        [[nodiscard]] const PhysicalDevice& PhysicalDevice() const { return m_physicalDevice; }
        [[nodiscard]] const std::unique_ptr<Core::Queue>& Queue(Queue::Type type) const;
        [[nodiscard]] const vk::CommandPool& CommandPool(Queue::Type type) const;

        void QuerySurfaceCapabilities() const;

        [[nodiscard]] std::optional<uint32_t> FindMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) const;

        vk::CommandBuffer BeginSingleTimeCommands(Queue::Type type) const;
        void EndSingleTimeCommands(vk::CommandBuffer commandBuffer, Queue::Type type) const;
    private:
        vk::Device m_device;
        Core::PhysicalDevice& m_physicalDevice;

        std::vector<Core::QueueFamily> m_queueFamilies;
        std::unordered_map<Core::Queue::Type, std::unique_ptr<Core::Queue>> m_queues;
        std::unordered_map<Core::Queue::Type, vk::CommandPool> m_commandPools;
    };
}
