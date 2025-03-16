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

inline static std::thread::id mainThreadId = std::this_thread::get_id();

namespace Core {
    Queue::Family::Family(const uint32_t index, const vk::QueueFamilyProperties &properties, const bool canPresent): m_index(index), m_properties(properties), m_canPresent(canPresent) {
        m_remainingQueues = properties.queueCount;
    }

    std::unique_ptr<Queue> Queue::Family::RequestQueue() {
        try {
            return std::make_unique<Queue>(*this);
        } catch (const std::runtime_error& err) {
            throw std::runtime_error("QueueFamily::RequestQueue : \n" + std::string(err.what()));
        }
    }

    std::unique_ptr<Queue> Queue::Family::RequestPresentQueue() {
        if (m_canPresent) {
            return std::make_unique<Queue>(*this);
        }
        throw std::runtime_error("QueueFamily::RequestPresentQueue : Queue family cannot present");
    }

    Queue::Queue(class Family &family): m_family(family) {
        if (m_family.m_remainingQueues == 0) {
            throw std::runtime_error("Queue::Queue : No more queues available in this family");
        }
        m_index = m_family.m_properties.queueCount - m_family.m_remainingQueues--;
        m_handle = GlobalDevice()->getQueue(m_family.Index(), m_index);
    }

    Queue::~Queue() {
        m_family.m_remainingQueues++;
    }

    CommandBuffer::CommandBuffer(const uint32_t familyIndex, const vk::CommandBuffer commandBuffer, const vk::CommandPool& parentCommandPool)
        : m_familyIndex(familyIndex), m_parentPool(parentCommandPool) {
        m_handle = commandBuffer;
        m_signalSemaphore = Core::GlobalDevice()->createSemaphore({});
        m_fence = Core::GlobalDevice()->createFence(vk::FenceCreateInfo().setFlags(vk::FenceCreateFlagBits::eSignaled));
    }

    CommandBuffer::~CommandBuffer() {
        Core::GlobalDevice().FreeCommandBuffer(*this);

        Core::GlobalDevice()->destroySemaphore(m_signalSemaphore);
        Core::GlobalDevice()->destroyFence(m_fence);
    }

    Device::Device(const CreateInfo& createInfo) : m_runtime(createInfo.runtime) {
        g_device = this;

        const auto& physicalDevice = m_runtime.PhysicalDevice();
        for (const auto& queueFamily : physicalDevice.QueueFamilyProperties()) {
            const auto queueFamilyIndex = static_cast<uint32_t>(&queueFamily - physicalDevice.QueueFamilyProperties().data());
            const bool canPresent = physicalDevice->getSurfaceSupportKHR(queueFamilyIndex, physicalDevice.Surface());

            m_queueFamilies.emplace_back(queueFamilyIndex, queueFamily, canPresent);
        }

        std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
        std::vector<std::vector<float>> queuePriorities;
        for (const auto& queueFamily : m_queueFamilies) {
            queuePriorities.emplace_back(queueFamily.Properties().queueCount, 1.0f);
            const auto queueCreateInfo = vk::DeviceQueueCreateInfo()
                .setQueueFamilyIndex(queueFamily.Index())
                .setQueuePriorities(queuePriorities.back());
            queueCreateInfos.emplace_back(queueCreateInfo);
        }

        auto deviceMeshShaderFeatures = vk::PhysicalDeviceMeshShaderFeaturesEXT()
            .setTaskShader(false)
            .setMeshShader(true);

        auto maintenance4Features = vk::PhysicalDeviceMaintenance4Features()
            .setMaintenance4(true)
            .setPNext(&deviceMeshShaderFeatures);

        const auto deviceCreateInfo = vk::DeviceCreateInfo()
            .setQueueCreateInfos(queueCreateInfos)
            .setPEnabledFeatures(&m_runtime.m_deviceFeatures)
            .setPNext(&maintenance4Features)
            .setPEnabledExtensionNames(m_runtime.m_deviceExtensions)
            .setPEnabledLayerNames(m_runtime.m_deviceLayers);

        m_handle = physicalDevice->createDevice(deviceCreateInfo);

        for (const auto& queueFamily : m_queueFamilies) {
            m_commandPools[queueFamily.Index()] = {};
        }
        CreateCommandPools(0);
    }

    Device::~Device() {
        FreeCommandPools(0);
        m_handle.destroy();
    }

    void Device::CreateCommandPools(const uint32_t threadId) {
        for (const auto& queueFamily : m_queueFamilies) {
            const auto commandPoolCreateInfo = vk::CommandPoolCreateInfo()
                .setFlags(vk::CommandPoolCreateFlagBits::eTransient | vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
                .setQueueFamilyIndex(queueFamily.Index());
            m_commandPools[queueFamily.Index()].emplace(threadId, m_handle.createCommandPool(commandPoolCreateInfo));
        }
    }

    void Device::FreeCommandPools(const uint32_t threadId) {
        for (const auto& commandPool : m_commandPools | std::views::values) {
            m_handle.destroyCommandPool(commandPool.at(threadId));
        }
        for (const auto& queueFamily : m_queueFamilies) {
            m_commandPools[queueFamily.Index()].erase(threadId);
        }
    }

    std::unique_ptr<Queue> Device::RequestQueue(const vk::QueueFlags type) {
        for (auto& queueFamily : m_queueFamilies) {
            if (!(queueFamily.Properties().queueFlags & type)) {
                continue;
            }
            try {
                return queueFamily.RequestQueue();
            } catch (const std::runtime_error&) {
                continue;
            }
        }
        throw std::runtime_error("Queue::RequestQueue : Failed to find queue with requested flags");
    }

    std::unique_ptr<Queue> Device::RequestPresentQueue() {
        for (auto& queueFamily : m_queueFamilies) {
            try {
                return queueFamily.RequestPresentQueue();
            } catch (const std::runtime_error&) {
                continue;
            }
        }
        throw std::runtime_error("Device::RequestPresentQueue: Failed to find suitable present queue!");
    }

    std::unique_ptr<CommandBuffer> Device::RequestCommandBuffer(const uint32_t familyIndex, const uint32_t thread) const {
        const auto& commandPool = m_commandPools.at(familyIndex).at(thread);
        const auto commandBufferAllocInfo = vk::CommandBufferAllocateInfo()
            .setCommandPool(commandPool)
            .setLevel(vk::CommandBufferLevel::ePrimary)
            .setCommandBufferCount(1);
        const auto commandBuffers = m_handle.allocateCommandBuffers(commandBufferAllocInfo);
        return std::make_unique<CommandBuffer>(familyIndex, commandBuffers.front(), commandPool);
    }

    void Device::FreeCommandBuffer(const CommandBuffer &commandBuffer) const {
        m_handle.freeCommandBuffers(commandBuffer.ParentPool(), *commandBuffer);
    }

    const PhysicalDevice& Device::QuerySurfaceCapabilities() const {
        auto& physicalDevice = m_runtime.PhysicalDevice();
        physicalDevice.QuerySurfaceCapabilities();
        return physicalDevice;
    }

    std::optional<uint32_t> Device::FindMemoryType(const uint32_t typeFilter, const vk::MemoryPropertyFlags properties) const {
        const auto& physicalDevice = m_runtime.PhysicalDevice();
        const auto memoryTypes = physicalDevice->getMemoryProperties().memoryTypes;
        for (uint32_t i = 0; i < memoryTypes.size(); i++) {
            if (typeFilter & 1 << i &&
                (memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }

        std::cerr << "Failed to find suitable memory type!" << std::endl;
        return std::nullopt;
    }

    void Device::RunSingleTimeCommand(const std::function<void(const Core::CommandBuffer&)> &command, const vk::QueueFlags requiredFlags,
        const vk::Fence fence, vk::Semaphore waitSemaphore, vk::Semaphore signalSemaphore) {
        const auto threadId = std::this_thread::get_id();
        uint32_t thread = threadId._Get_underlying_id();
        if (threadId == mainThreadId) {
            thread = 0;
        }

        const auto queue = RequestQueue(requiredFlags);
        const auto commandBuffer = RequestCommandBuffer(queue->Family().Index(), thread);

        (*commandBuffer)->begin(vk::CommandBufferBeginInfo().setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
        command(*commandBuffer);
        (*commandBuffer)->end();

        const auto commandBuffers = std::array { **commandBuffer };
        const auto dstStageMask = std::vector<vk::PipelineStageFlags> { vk::PipelineStageFlagBits::eAllCommands };
        auto submitInfo = vk::SubmitInfo()
            .setCommandBuffers(commandBuffers);

        if (waitSemaphore != nullptr)
            submitInfo
                .setWaitSemaphores(waitSemaphore)
                .setWaitDstStageMask(dstStageMask);

        if (signalSemaphore != nullptr)
            submitInfo.setSignalSemaphores(signalSemaphore);

        try {
            (*queue)->submit(submitInfo, fence);
        } catch (const std::runtime_error& e) {
            std::cerr << e.what() << std::endl;
        }
        (*queue)->waitIdle();
    }
}

