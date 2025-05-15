//
// Created by radue on 10/13/2024.
//

#pragma once

#include <memory>
#include <unordered_set>

#include <vulkan/vulkan.hpp>

#include "core/window.h"
#include "core/physicalDevice.h"

namespace Coral::Core {
    class Runtime {
        friend class PhysicalDevice;
        friend class Device;
    public:
        struct CreateInfo {
            const Window &window;
            vk::PhysicalDeviceFeatures deviceFeatures;
            std::vector<const char*> instanceLayers;
            std::vector<const char*> instanceExtensions;
            std::vector<const char*> deviceExtensions;
            std::vector<const char*> deviceLayers;
            std::unordered_set<vk::QueueFlagBits> requiredQueueFamilies;
        };

        explicit Runtime(const CreateInfo &createInfo);
        ~Runtime();
        Runtime(const Runtime &) = delete;
        Runtime &operator=(const Runtime &) = delete;

        void CreateInstance();
        void SelectPhysicalDevice();

        void SetupDebugMessenger();
        void destroyDebugMessenger() const;

        [[nodiscard]] const vk::Instance& Instance() const { return m_instance; }
        [[nodiscard]] const vk::SurfaceKHR& Surface() const { return m_surface; }
        [[nodiscard]] PhysicalDevice& PhysicalDevice() const { return *m_physicalDevice; }

    private:
        const Window &m_window;

        vk::PhysicalDeviceFeatures m_deviceFeatures;
        std::vector<const char*> m_instanceLayers;
        std::vector<const char*> m_instanceExtensions;
        std::vector<const char*> m_deviceExtensions;
        std::vector<const char*> m_deviceLayers;
        std::unordered_set<vk::QueueFlagBits> m_requiredQueueFamilies;

        vk::Instance m_instance;
        vk::DebugUtilsMessengerEXT m_debugMessenger;
        vk::SurfaceKHR m_surface;
        std::vector<vk::PhysicalDevice> m_physicalDevices;
        std::unique_ptr<Core::PhysicalDevice> m_physicalDevice = nullptr;
    };
}