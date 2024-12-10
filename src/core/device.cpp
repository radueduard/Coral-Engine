//
// Created by radue on 10/14/2024.
//

#include "device.h"

#include <iostream>
#include <ranges>
#include <thread>
#include <unordered_map>

#include "physicalDevice.h"
#include "runtime.h"

namespace Core {
    Queue::Queue(const Device& device, const uint32_t familyIndex, const uint32_t index)
        : familyIndex(familyIndex), index(index) {
        queue = (*device).getQueue(familyIndex, index);
    }

    void Device::Init() {
        m_instance = std::make_unique<Device>();
    }

    void Device::Destroy() {
        m_instance.reset();
    }

    std::unique_ptr<Device> Device::m_instance = nullptr;

    Device::Device() {
        const auto& physicalDevice = Core::Runtime::Get().PhysicalDevice();
        for (const auto& queueFamily : physicalDevice.QueueFamilyProperties()) {
            const auto queueFamilyIndex = static_cast<uint32_t>(&queueFamily - physicalDevice.QueueFamilyProperties().data());
            const bool canPresent = (*physicalDevice).getSurfaceSupportKHR(queueFamilyIndex, physicalDevice.Surface());

            m_queueFamilies.emplace_back(QueueFamily {
                .index = queueFamilyIndex,
                .properties = queueFamily,
                .presentSupport = canPresent,
            });
        }

        std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
        std::vector<std::vector<float>> queuePriorities;
        for (const auto& queueFamily : m_queueFamilies) {
            queuePriorities.emplace_back(queueFamily.properties.queueCount, 1.0f);
            const auto queueCreateInfo = vk::DeviceQueueCreateInfo()
                .setQueueFamilyIndex(queueFamily.index)
                .setQueuePriorities(queuePriorities.back());
            queueCreateInfos.emplace_back(queueCreateInfo);
        }

        auto deviceMeshShaderFeatures = vk::PhysicalDeviceMeshShaderFeaturesEXT()
            .setTaskShader(false)
            .setMeshShader(true);

        auto maintenance4Features = vk::PhysicalDeviceMaintenance4Features()
            .setMaintenance4(true)
            .setPNext(&deviceMeshShaderFeatures);

        const auto shaderBufferFloat32Atomics = vk::PhysicalDeviceShaderAtomicFloatFeaturesEXT()
            .setShaderBufferFloat32Atomics(true)
            .setPNext(&maintenance4Features);

        const auto deviceCreateInfo = vk::DeviceCreateInfo()
            .setQueueCreateInfos(queueCreateInfos)
            .setPEnabledFeatures(&Runtime::settings.deviceFeatures)
            .setPNext(&shaderBufferFloat32Atomics)
            .setPEnabledExtensionNames(Runtime::settings.deviceExtensions)
            .setPEnabledLayerNames(Runtime::settings.deviceLayers);

        m_device = (*physicalDevice).createDevice(deviceCreateInfo);

        for (const auto& queueFamily : m_queueFamilies) {
            for (uint32_t i = 0; i < queueFamily.properties.queueCount; i++) {
                auto q = std::make_unique<Queue>(*this, queueFamily.index, i);
                m_queueFamilies[queueFamily.index].queues.emplace_back(std::move(q));
            }
        }

        for (const auto& queueFamily : m_queueFamilies) {
            m_commandPools[queueFamily.index] = {};
            for (uint32_t i = 0; i < std::thread::hardware_concurrency(); i++) {
                const auto commandPoolCreateInfo = vk::CommandPoolCreateInfo()
                    .setFlags(vk::CommandPoolCreateFlagBits::eTransient | vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
                    .setQueueFamilyIndex(queueFamily.index);
                m_commandPools[queueFamily.index].emplace_back(m_device.createCommandPool(commandPoolCreateInfo));
            }
        }
    }

    Device::~Device() {
        for (const auto &commandPool: m_commandPools | std::views::values) {
            for (const auto &pool: commandPool) {
                m_device.destroyCommandPool(pool);
            }
        }
        m_device.destroy();
    }

    std::optional<Queue*> Device::RequestQueue(const vk::QueueFlags type) const {
        for (const auto& queueFamily : m_queueFamilies) {
            if (!(queueFamily.properties.queueFlags & type)) {
                continue;
            }
            for (uint32_t i = 0; i < queueFamily.properties.queueCount; i++) {
                auto queue = queueFamily.queues[i].get();
                if (queue->inUse) {
                    continue;
                }
                queue->inUse = true;
                return queue;
            }
        }
        return std::nullopt;
    }

    std::optional<Queue*> Device::RequestPresentQueue() const {
        for (const auto& queueFamily : m_queueFamilies) {
            if (queueFamily.presentSupport) {
                for (uint32_t i = 0; i < queueFamily.properties.queueCount; i++) {
                    auto queue = queueFamily.queues[i].get();
                    if (queue->inUse) {
                        continue;
                    }
                    queue->inUse = true;
                    return queue;
                }
            }
        }
        return std::nullopt;
    }

    uint32_t Device::GetQueueFamilyIndex(const vk::QueueFlags type) const {
        for (const auto& queueFamily : m_queueFamilies) {
            if (queueFamily.properties.queueFlags & type) {
                return queueFamily.index;
            }
        }
        return -1;
    }

    std::vector<CommandBuffer> Device::RequestCommandBuffers(const uint32_t familyIndex, const uint32_t count, const uint32_t thread) const {
        const auto computeBufferAllocInfo = vk::CommandBufferAllocateInfo()
            .setCommandPool(m_commandPools.at(familyIndex)[thread])
            .setLevel(vk::CommandBufferLevel::ePrimary)
            .setCommandBufferCount(count);
        const auto commandBuffers = m_device.allocateCommandBuffers(computeBufferAllocInfo);
        std::vector<CommandBuffer> result;
        for (const auto& commandBuffer : commandBuffers) {
            result.emplace_back(familyIndex, commandBuffer);
        }
        return result;
    }

    void Device::FreeCommandBuffers(const std::vector<CommandBuffer> &commandBuffers, const uint32_t thread) const {
        for (const auto&[familyIndex, commandBuffer] : commandBuffers) {
            m_device.freeCommandBuffers(m_commandPools.at(familyIndex)[thread], commandBuffer);
        }
    }

    void Device::QuerySurfaceCapabilities() const {
        Core::Runtime::Get().PhysicalDevice().QuerySurfaceCapabilities();
    }

    std::optional<uint32_t> Device::FindMemoryType(const uint32_t typeFilter, const vk::MemoryPropertyFlags properties) const {
        const auto& physicalDevice = Core::Runtime::Get().PhysicalDevice();
        const auto memoryTypes = (*physicalDevice).getMemoryProperties().memoryTypes;
        for (uint32_t i = 0; i < memoryTypes.size(); i++) {
            if ((typeFilter & (1 << i)) &&
                (memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }

        std::cerr << "Failed to find suitable memory type!" << std::endl;
        return std::nullopt;
    }

    void Device::RunSingleTimeCommand(const std::function<void(vk::CommandBuffer)> &command, const vk::QueueFlags requiredFlags, const vk::Fence fence, const uint32_t thread) const {
        const auto maybeQueue = RequestQueue(requiredFlags);
        if (!maybeQueue.has_value()) {
            std::cerr << "Failed to find suitable queue for single time commands!" << std::endl;
            return;
        }
        const auto queue = maybeQueue.value();

        const auto allocInfo = vk::CommandBufferAllocateInfo()
            .setCommandPool(m_commandPools.at(queue->familyIndex)[thread])
            .setLevel(vk::CommandBufferLevel::ePrimary)
            .setCommandBufferCount(1);

        const auto commandBuffer = m_device.allocateCommandBuffers(allocInfo).front();

        commandBuffer.begin(vk::CommandBufferBeginInfo().setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
        command(commandBuffer);
        commandBuffer.end();

        const auto submitInfo = vk::SubmitInfo()
            .setCommandBuffers(commandBuffer);

        queue->queue.submit(submitInfo, fence);
        queue->queue.waitIdle();
        queue->inUse = false;
    }
}

