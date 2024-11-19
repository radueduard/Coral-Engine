//
// Created by radue on 10/13/2024.
//

#include "runtime.h"

#include <iostream>

#include "../extensions/debugUtils.h"
#include "../extensions/meshShader.h"

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
            void *pUserData) {

    switch (messageSeverity) {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            std::cerr << pCallbackData->pMessage << std::endl;
        return VK_FALSE;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            std::cerr << pCallbackData->pMessage << std::endl;
        return VK_TRUE;
        default:
            return VK_FALSE;
    }
}

namespace Core {
    Runtime::Settings Runtime::settings = {
        .deviceFeatures = vk::PhysicalDeviceFeatures()
            .setSamplerAnisotropy(true)
            .setFragmentStoresAndAtomics(true)
            .setVertexPipelineStoresAndAtomics(true)
        ,
        .instanceLayers = {
            "VK_LAYER_KHRONOS_validation",
        },
        .instanceExtensions = {
            VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
        },
        .deviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            VK_EXT_MESH_SHADER_EXTENSION_NAME,
        },
        .deviceLayers = {
            "VK_LAYER_KHRONOS_validation",
        },
        .requiredQueueFamilies = {
            vk::QueueFlagBits::eGraphics,
            vk::QueueFlagBits::eCompute,
            vk::QueueFlagBits::eTransfer,
        },
    };

    Runtime::Runtime(const Window &window) : m_window(window) {
        createInstance();

        Ext::DebugUtils::importFunctions(m_instance);
        Ext::MeshShader::importFunctions(m_instance);

        setupDebugMessenger();
        selectPhysicalDevice();
    }

    Runtime::~Runtime() {
        m_physicalDevice.reset();
        m_instance.destroySurfaceKHR(m_surface);
        destroyDebugMessenger();
        m_instance.destroy();
    }

    void Runtime::createInstance() {
        constexpr auto appInfo = vk::ApplicationInfo()
            .setPApplicationName("Vulkan Application")
            .setApplicationVersion(VK_MAKE_VERSION(1, 0, 0))
            .setPEngineName("Vulkan Graphics Engine")
            .setEngineVersion(VK_MAKE_VERSION(1, 0, 0))
            .setApiVersion(VK_API_VERSION_1_3);

        const auto windowExtensions = Window::GetRequiredExtensions();
        settings.instanceExtensions.insert(settings.instanceExtensions.end(), windowExtensions.begin(), windowExtensions.end());

        const auto createInfo = vk::InstanceCreateInfo()
            .setPApplicationInfo(&appInfo)
            .setPEnabledExtensionNames(settings.instanceExtensions)
            .setPEnabledLayerNames(settings.instanceLayers);;

        m_instance = vk::createInstance(createInfo);
    }

    void Runtime::setupDebugMessenger() {
        const auto debugCreateInfo = vk::DebugUtilsMessengerCreateInfoEXT()
            .setMessageSeverity(
                vk::DebugUtilsMessageSeverityFlagBitsEXT::eError |
                vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose)
                // vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo)
            .setMessageType(
                vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance)
            .setPfnUserCallback(debugCallback);

        Ext::DebugUtils::createDebugUtilsMessengerEXT(m_instance, debugCreateInfo, nullptr, &m_debugMessenger);
    }

    void Runtime::destroyDebugMessenger() const {
        Ext::DebugUtils::destroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
    }

    void Runtime::selectPhysicalDevice() {
        m_surface = m_window.CreateSurface(m_instance);
        for (const auto devices = m_instance.enumeratePhysicalDevices(); const auto &device : devices) {
            if (auto physicalDevice = std::make_unique<Core::PhysicalDevice>(device, m_surface); physicalDevice->isSuitable()) {
                m_physicalDevice = std::move(physicalDevice);
                break;
            }
        }
    }

} // namespace Core