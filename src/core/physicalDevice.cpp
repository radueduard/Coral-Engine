//
// Created by radue on 10/14/2024.
//

#include <iostream>

#include "physicalDevice.h"
#include "runtime.h"

namespace Coral::Core {
    PhysicalDevice::PhysicalDevice(const CreateInfo& createInfo)
        : m_runtime(createInfo.runtime), m_surface(createInfo.surface)
    {
        m_handle = createInfo.physicalDevice;
        m_properties = m_handle.getProperties();
        m_features = m_handle.getFeatures();
        m_memoryProperties = m_handle.getMemoryProperties();
        m_extensionProperties = m_handle.enumerateDeviceExtensionProperties();
        m_queueFamilyProperties = m_handle.getQueueFamilyProperties();

        QuerySurfaceCapabilities();
    }

    void PhysicalDevice::QuerySurfaceCapabilities() {
        m_capabilities = m_handle.getSurfaceCapabilitiesKHR(m_surface);
        m_formats = m_handle.getSurfaceFormatsKHR(m_surface);
        m_presentModes = m_handle.getSurfacePresentModesKHR(m_surface);
    }


    bool PhysicalDevice::isSuitable() const {
        auto requiredExtensions = std::unordered_set<std::string>(m_runtime.m_deviceExtensions.begin(), m_runtime.m_deviceExtensions.end());

        return hasRequiredQueueFamilies(m_runtime.m_requiredQueueFamilies)
            && hasRequiredExtensions(requiredExtensions)
            && hasRequiredFeatures(m_runtime.m_deviceFeatures);
    }

    bool PhysicalDevice::hasRequiredQueueFamilies(const std::unordered_set<vk::QueueFlagBits>& requiredQueueFamilies) const {
        VkBool32 presentSupported = false;
        std::unordered_set<vk::QueueFlagBits> supportedQueueFamilies {};

        for (const auto& queueFamily : m_queueFamilyProperties) {
            const auto queueFamilyIndex = static_cast<uint32_t>(&queueFamily - m_queueFamilyProperties.data());
            for (const auto& requiredQueueFamily : requiredQueueFamilies) {
                if (queueFamily.queueFlags & requiredQueueFamily) {
                    supportedQueueFamilies.insert(requiredQueueFamily);
                }
            }
            presentSupported |= m_handle.getSurfaceSupportKHR(queueFamilyIndex, m_surface);
        }

        return requiredQueueFamilies == supportedQueueFamilies && presentSupported;
    }

    bool PhysicalDevice::hasRequiredExtensions(std::unordered_set<std::string>& requiredExtensions) const {
        for (const auto& availableExtension : m_extensionProperties) {
            std::string extensionName = availableExtension.extensionName;
            requiredExtensions.erase(extensionName);
        }
        for (const auto& extension : requiredExtensions) {
            std::cerr << "PhysicalDevice : Missing required extension " << extension << std::endl;
        }

        return requiredExtensions.empty();
    }

    bool PhysicalDevice::hasRequiredFeatures(const vk::PhysicalDeviceFeatures& requiredFeatures) const {
        const auto requiredFeaturesPtr = reinterpret_cast<const vk::Bool32*>(&requiredFeatures);
        const auto deviceFeaturesPtr = reinterpret_cast<const vk::Bool32*>(&m_features);

        for (size_t i = 0; i < sizeof(vk::PhysicalDeviceFeatures) / sizeof(vk::Bool32); ++i) {
            if (requiredFeaturesPtr[i] && !deviceFeaturesPtr[i]) {
                return false;
            }
        }

        return true;
    }
}