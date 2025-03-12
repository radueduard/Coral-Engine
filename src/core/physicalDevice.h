//
// Created by radue on 10/14/2024.
//
#pragma once

#include <unordered_set>
#include <vulkan/vulkan.hpp>

#include "utils/globalWrapper.h"

namespace Core {
    class Runtime;
}

namespace Core {
    class PhysicalDevice final : public EngineWrapper<vk::PhysicalDevice> {
        friend class Runtime;
    public:
        struct CreateInfo {
            const Runtime& runtime;
            vk::PhysicalDevice physicalDevice;
            vk::SurfaceKHR surface;
        };

        explicit PhysicalDevice(const CreateInfo& createInfo);
        ~PhysicalDevice() override = default;
        PhysicalDevice(const PhysicalDevice &) = delete;
        PhysicalDevice &operator=(const PhysicalDevice &) = delete;

        [[nodiscard]] bool isSuitable() const;
        [[nodiscard]] const std::vector<vk::QueueFamilyProperties>& QueueFamilyProperties() const { return m_queueFamilyProperties; }

        [[nodiscard]] const vk::SurfaceKHR& Surface() const { return m_surface; }
        [[nodiscard]] const vk::SurfaceCapabilitiesKHR& SurfaceCapabilities() const { return m_capabilities; }
        [[nodiscard]] const std::vector<vk::SurfaceFormatKHR>& SurfaceFormats() const { return m_formats; }
        [[nodiscard]] const std::vector<vk::PresentModeKHR>& SurfacePresentModes() const { return m_presentModes; }

        void QuerySurfaceCapabilities();
    private:
        [[nodiscard]] bool hasRequiredQueueFamilies(const std::unordered_set<vk::QueueFlagBits>&) const;
        [[nodiscard]] bool hasRequiredExtensions(std::unordered_set<std::string>& requiredExtensions) const;
        [[nodiscard]] bool hasRequiredFeatures(const vk::PhysicalDeviceFeatures& requiredFeatures) const;
        [[nodiscard]] bool isSwapChainSupported() const { return !m_formats.empty() && !m_presentModes.empty(); }

        const Runtime& m_runtime;

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
}