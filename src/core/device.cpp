//
// Created by radue on 10/14/2024.
//

#include "device.h"

#include <iostream>
#include <ranges>
#include <unordered_map>

#include "runtime.h"

namespace Core {
    Queue::Type Queue::TypeFromFlag(const vk::QueueFlagBits queueFlag) {
        switch (queueFlag) {
            case vk::QueueFlagBits::eGraphics:
                return Type::Graphics;
            case vk::QueueFlagBits::eCompute:
                return Type::Compute;
            case vk::QueueFlagBits::eTransfer:
                return Type::Transfer;
            default:
                return Type::Unassigned;
        }
    }

    Device::Device(Core::PhysicalDevice &physicalDevice) : m_physicalDevice(physicalDevice) {

        for (const auto& queueFamily : physicalDevice.QueueFamilyProperties()) {
            const auto queueFamilyIndex = static_cast<uint32_t>(&queueFamily - physicalDevice.QueueFamilyProperties().data());
            bool canPresent = (*physicalDevice).getSurfaceSupportKHR(queueFamilyIndex, physicalDevice.Surface());

            const Core::QueueFamily qF = {
                .index = queueFamilyIndex,
                .properties = queueFamily,
                .presentSupport = canPresent,
                .remainingQueues = queueFamily.queueCount
            };

            m_queueFamilies.emplace_back(qF);
        }

        for (auto& queueFamily : m_queueFamilies) {

            for (const auto& queueFlag : Runtime::settings.requiredQueueFamilies) {
                if (queueFamily.remainingQueues == 0) { break; }
                if (queueFamily.properties.queueFlags & queueFlag) {
                    auto queue = std::make_unique<Core::Queue>(Core::Queue{
                        .familyIndex = queueFamily.index,
                        .index = queueFamily.properties.queueCount - queueFamily.remainingQueues,
                        .type = Queue::TypeFromFlag(queueFlag)
                    });
                    m_queues[Queue::TypeFromFlag(queueFlag)] = std::move(queue);
                    queueFamily.remainingQueues--;
                }
            }
        }

        for (auto& queueFamily : m_queueFamilies) {
            if (queueFamily.remainingQueues == 0) { continue; }
            if (queueFamily.presentSupport) {
                auto queue = std::make_unique<Core::Queue>(Core::Queue{
                    .familyIndex = queueFamily.index,
                    .index = queueFamily.properties.queueCount - queueFamily.remainingQueues,
                    .type = Queue::Type::Present
                });
                m_queues[Queue::Type::Present] = std::move(queue);
                queueFamily.remainingQueues--;
            }
        }

        std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
        for (const auto& queueFamily : m_queueFamilies) {
            if (queueFamily.properties.queueCount == queueFamily.remainingQueues) { continue; }
            int queueCount = queueFamily.properties.queueCount - queueFamily.remainingQueues;
            const auto* queuePriorities = new float[queueCount]{ 1.0f };

            const auto queueCreateInfo = vk::DeviceQueueCreateInfo()
                .setQueueFamilyIndex(queueFamily.index)
                .setQueueCount(queueCount)
                .setPQueuePriorities(queuePriorities);
            queueCreateInfos.emplace_back(queueCreateInfo);
        }

        auto deviceMeshShaderFeatures = vk::PhysicalDeviceMeshShaderFeaturesEXT()
            .setTaskShader(false)
            .setMeshShader(true);

        const auto maintenance4Features = vk::PhysicalDeviceMaintenance4Features()
            .setMaintenance4(true)
            .setPNext(&deviceMeshShaderFeatures);

        const auto deviceCreateInfo = vk::DeviceCreateInfo()
            .setQueueCreateInfos(queueCreateInfos)
            .setPEnabledFeatures(&Runtime::settings.deviceFeatures)
            .setPNext(&maintenance4Features)
            .setPEnabledExtensionNames(Runtime::settings.deviceExtensions)
            .setPEnabledLayerNames(Runtime::settings.deviceLayers);

        m_device = (*physicalDevice).createDevice(deviceCreateInfo);

        for (const auto& queueFamilyFlag : Runtime::settings.requiredQueueFamilies) {
            m_queues[Queue::TypeFromFlag(queueFamilyFlag)]->queue =
                m_device.getQueue(m_queues[Queue::TypeFromFlag(queueFamilyFlag)]->familyIndex,
                                  m_queues[Queue::TypeFromFlag(queueFamilyFlag)]->index);
        }
        m_queues[Queue::Type::Present]->queue =
            m_device.getQueue(m_queues[Queue::Type::Present]->familyIndex,
                              m_queues[Queue::Type::Present]->index);

        for (const auto& [type, queue] : m_queues) {
            const auto commandPoolCreateInfo = vk::CommandPoolCreateInfo()
                .setFlags(vk::CommandPoolCreateFlagBits::eTransient | vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
                .setQueueFamilyIndex(queue->familyIndex);
            m_commandPools[type] = m_device.createCommandPool(commandPoolCreateInfo);
        }
    }

    Device::~Device() {
        for (const auto &commandPool: m_commandPools | std::views::values) {
            m_device.destroyCommandPool(commandPool);
        }
        m_device.destroy();
    }

    const std::unique_ptr<Core::Queue>& Device::Queue(const Queue::Type type) const {
        return m_queues.at(type);
    }

    const vk::CommandPool &Device::CommandPool(const Queue::Type type) const {
        return m_commandPools.at(type);
    }

    void Device::QuerySurfaceCapabilities() const {
        m_physicalDevice.QuerySurfaceCapabilities();
    }

    std::optional<uint32_t> Device::FindMemoryType(const uint32_t typeFilter, const vk::MemoryPropertyFlags properties) const {
        const auto memoryTypes = (*m_physicalDevice).getMemoryProperties().memoryTypes;
        for (uint32_t i = 0; i < memoryTypes.size(); i++) {
            if ((typeFilter & (1 << i)) &&
                (memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }

        std::cerr << "Failed to find suitable memory type!" << std::endl;
        return std::nullopt;
    }

    vk::CommandBuffer Device::BeginSingleTimeCommands(const Queue::Type type) const {
        const auto allocInfo = vk::CommandBufferAllocateInfo()
            .setCommandPool(m_commandPools.at(type))
            .setLevel(vk::CommandBufferLevel::ePrimary)
            .setCommandBufferCount(1);

        const auto commandBuffer = m_device.allocateCommandBuffers(allocInfo).front();
        commandBuffer.begin(vk::CommandBufferBeginInfo().setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));

        return commandBuffer;
    }

    void Device::EndSingleTimeCommands(vk::CommandBuffer commandBuffer, const Queue::Type type) const {
        commandBuffer.end();
        const auto submitInfo = vk::SubmitInfo()
            .setCommandBuffers(commandBuffer);

        const auto queue = m_queues.at(type)->queue;
        queue.waitIdle();
        queue.submit(submitInfo);
    }

}
