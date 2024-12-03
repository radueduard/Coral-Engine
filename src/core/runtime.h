//
// Created by radue on 10/13/2024.
//

#pragma once

#include <vulkan/vulkan.hpp>

#include "physicalDevice.h"
#include "window.h"

namespace Core {
    /**
     * @brief Singleton class for the Vulkan instance and physical device selection
     *
     */

    class Runtime {
    public:
        static struct Settings {
            vk::PhysicalDeviceFeatures deviceFeatures;
            std::vector<const char*> instanceLayers;
            std::vector<const char*> instanceExtensions;
            std::vector<const char*> deviceExtensions;
            std::vector<const char*> deviceLayers;
            std::unordered_set<vk::QueueFlagBits> requiredQueueFamilies;
        } settings;

        static void Init();
        static void Destroy();

        static Runtime& Get() { return *m_instance; }

        Runtime();
        ~Runtime();
        Runtime(const Runtime &) = delete;
        Runtime &operator=(const Runtime &) = delete;

        void createInstance();
        void selectPhysicalDevice();

        void setupDebugMessenger();
        void destroyDebugMessenger() const;

        [[nodiscard]] const vk::Instance& Instance() const { return m_vkInstance; }
        [[nodiscard]] const vk::SurfaceKHR& Surface() const { return m_surface; }
        [[nodiscard]] PhysicalDevice& PhysicalDevice() const { return *m_physicalDevice; }
    private:
        static std::unique_ptr<Runtime> m_instance;

        vk::Instance m_vkInstance;
        vk::DebugUtilsMessengerEXT m_debugMessenger;
        vk::SurfaceKHR m_surface;
        std::unique_ptr<Core::PhysicalDevice> m_physicalDevice = nullptr;
    };
}