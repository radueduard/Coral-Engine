//
// Created by radue on 10/14/2024.
//
#pragma once

#include <unordered_set>
#include <vulkan/vulkan.hpp>

namespace Core {
    /**
     * @brief Wrapper for VkPhysicalDevice and its properties
     */
    class PhysicalDevice {
        friend class Runtime;
    public:
        explicit PhysicalDevice(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface);
        ~PhysicalDevice() = default;
        PhysicalDevice(const PhysicalDevice &) = delete;
        PhysicalDevice &operator=(const PhysicalDevice &) = delete;

        vk::PhysicalDevice operator *() const { return m_physicalDevice; }

        [[nodiscard]] bool isSuitable() const;
        [[nodiscard]] const std::vector<vk::QueueFamilyProperties>& QueueFamilyProperties() const { return m_queueFamilyProperties; }

        // [[nodiscard]] const vk::PhysicalDeviceProperties& Properties() const { return m_properties; }
        // [[nodiscard]] const vk::PhysicalDeviceFeatures& Features() const { return m_features; }
        // [[nodiscard]] const vk::PhysicalDeviceMemoryProperties& MemoryProperties() const { return m_memoryProperties; }
        // [[nodiscard]] const std::vector<vk::ExtensionProperties>& ExtensionProperties() const { return m_extensionProperties; }

        [[nodiscard]] const vk::SurfaceKHR& Surface() const { return m_surface; }
        [[nodiscard]] const vk::SurfaceCapabilitiesKHR& SurfaceCapabilities() const { return m_capabilities; }
        [[nodiscard]] const std::vector<vk::SurfaceFormatKHR>& SurfaceFormats() const { return m_formats; }
        [[nodiscard]] const std::vector<vk::PresentModeKHR>& SurfacePresentModes() const { return m_presentModes; }

        void QuerySurfaceCapabilities();
    private:
        bool hasRequiredQueueFamilies(const std::unordered_set<vk::QueueFlagBits>&) const;
        bool hasRequiredExtensions(std::unordered_set<std::string>& requiredExtensions) const;
        bool hasRequiredFeatures(const vk::PhysicalDeviceFeatures& requiredFeatures) const;
        bool isSwapchainSupported() const { return !m_formats.empty() && !m_presentModes.empty(); }

        vk::PhysicalDevice m_physicalDevice;

        vk::PhysicalDeviceProperties m_properties;
        vk::PhysicalDeviceFeatures m_features;
        vk::PhysicalDeviceMemoryProperties m_memoryProperties;
        std::vector<vk::ExtensionProperties> m_extensionProperties;

        std::vector<vk::QueueFamilyProperties> m_queueFamilyProperties;

        vk::SurfaceKHR m_surface;

        vk::SurfaceCapabilitiesKHR m_capabilities;
        std::vector<vk::SurfaceFormatKHR> m_formats;
        std::vector<vk::PresentModeKHR> m_presentModes;
    };
} // namespace Core
