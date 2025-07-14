//
// Created by radue on 10/13/2024.
//

#include "runtime.h"

#include <iostream>

#include "physicalDevice.h"
#include "window.h"

#include "extensions/debugUtils.h"
#include "extensions/meshShader.h"

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {

    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}

namespace Coral::Core {
    Runtime::Runtime(const CreateInfo &createInfo) {
		s_runtime = this;

        m_deviceFeatures = createInfo.deviceFeatures;
        m_deviceExtensions = createInfo.deviceExtensions;
        m_deviceLayers = createInfo.deviceLayers;
        m_instanceExtensions = createInfo.instanceExtensions;
        m_instanceLayers = createInfo.instanceLayers;
        m_requiredQueueFamilies = createInfo.requiredQueueFamilies;

        CreateInstance();
        Ext::DebugUtils::ImportFunctions(m_instance);
        Ext::MeshShader::ImportFunctions(m_instance);

        SetupDebugMessenger();
        SelectPhysicalDevice();
    }

    Runtime::~Runtime() {
        m_physicalDevice.reset();
        m_instance.destroySurfaceKHR(m_surface);
        destroyDebugMessenger();
        m_instance.destroy();
    }

    void Runtime::CreateInstance() {
        constexpr auto appInfo = vk::ApplicationInfo()
            .setPApplicationName("Vulkan Application")
            .setApplicationVersion(VK_MAKE_VERSION(1, 0, 0))
            .setPEngineName("Vulkan Graphics Engine")
            .setEngineVersion(VK_MAKE_VERSION(1, 0, 0))
            .setApiVersion(VK_API_VERSION_1_3);

        const auto windowExtensions = Window::Get().GetRequiredExtensions();
        m_instanceExtensions.insert(m_instanceExtensions.end(), windowExtensions.begin(), windowExtensions.end());

        const auto createInfo = vk::InstanceCreateInfo()
            .setPApplicationInfo(&appInfo)
            .setPEnabledExtensionNames(m_instanceExtensions)
            .setPEnabledLayerNames(m_instanceLayers);

        m_instance = vk::createInstance(createInfo);
    }

    void Runtime::SetupDebugMessenger() {
        const auto debugCreateInfo = vk::DebugUtilsMessengerCreateInfoEXT()
            .setMessageSeverity(
                vk::DebugUtilsMessageSeverityFlagBitsEXT::eError |
                vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose)
                // vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo)
            .setMessageType(
                vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation)
            .setPfnUserCallback(reinterpret_cast<vk::PFN_DebugUtilsMessengerCallbackEXT>(debugCallback));

        Ext::DebugUtils::createDebugUtilsMessengerEXT(m_instance, debugCreateInfo, nullptr, &m_debugMessenger);
    }

    void Runtime::destroyDebugMessenger() const {
        Ext::DebugUtils::destroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
    }

    void Runtime::SelectPhysicalDevice() {
        m_surface = Window::Get().CreateSurface(m_instance);
        m_physicalDevices = m_instance.enumeratePhysicalDevices();
        for (const auto physicalDeviceCandidate : m_physicalDevices) {
            const PhysicalDevice::CreateInfo createInfo = {
                .runtime = *this,
                .physicalDevice = physicalDeviceCandidate,
                .surface = m_surface,
            };

            if (auto physicalDevice = std::make_unique<Core::PhysicalDevice>(createInfo); physicalDevice->isSuitable()) {
                if (physicalDevice->m_properties.deviceType != vk::PhysicalDeviceType::eDiscreteGpu) {
                    continue;
                }

                // Print physical device information
                std::cout << "Selected physical device: " << physicalDevice->m_properties.deviceName << std::endl;
                std::cout << "Device type: " << vk::to_string(physicalDevice->m_properties.deviceType) << std::endl;

                m_physicalDevice = std::move(physicalDevice);
                return;
            }
        }
        if (!m_physicalDevice) {
            throw std::runtime_error("Failed to find a suitable physical device!");
        }
    }
}