//
// Created by radue on 10/14/2024.
//

#include "physicalDevice.h"

#include <iostream>

#include "runtime.h"

namespace Core {
    PhysicalDevice::PhysicalDevice(const vk::PhysicalDevice physicalDevice, const vk::SurfaceKHR surface)
        : m_physicalDevice(physicalDevice), m_surface(surface)
    {
        m_properties = physicalDevice.getProperties();
        m_features = physicalDevice.getFeatures();
        m_memoryProperties = physicalDevice.getMemoryProperties();
        m_extensionProperties = physicalDevice.enumerateDeviceExtensionProperties();
        m_queueFamilyProperties = physicalDevice.getQueueFamilyProperties();

        QuerySurfaceCapabilities();
    }

    void PhysicalDevice::QuerySurfaceCapabilities() {
        m_capabilities = m_physicalDevice.getSurfaceCapabilitiesKHR(m_surface);
        m_formats = m_physicalDevice.getSurfaceFormatsKHR(m_surface);
        m_presentModes = m_physicalDevice.getSurfacePresentModesKHR(m_surface);
    }


    bool PhysicalDevice::isSuitable() const {
        auto requiredExtensions = std::unordered_set<std::string>(Runtime::settings.deviceExtensions.begin(), Runtime::settings.deviceExtensions.end());

        return hasRequiredQueueFamilies(Runtime::settings.requiredQueueFamilies)
            && hasRequiredExtensions(requiredExtensions)
            && hasRequiredFeatures(Runtime::settings.deviceFeatures);
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
            presentSupported |= m_physicalDevice.getSurfaceSupportKHR(queueFamilyIndex, m_surface);
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
} // namespace Core