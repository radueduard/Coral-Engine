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
            .setFillModeNonSolid(true)
            .setVertexPipelineStoresAndAtomics(true)
        ,
        .instanceLayers = {
            "VK_LAYER_KHRONOS_validation",
        },
        .instanceExtensions = {
            VK_EXT_DEBUG_UTILS_EXTENSION_NAME
        },
        .deviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            VK_EXT_MESH_SHADER_EXTENSION_NAME,
            VK_EXT_SHADER_ATOMIC_FLOAT_EXTENSION_NAME,
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

    Runtime::Runtime() {
        createInstance();
        Ext::DebugUtils::importFunctions(m_vkInstance);
        Ext::MeshShader::importFunctions(m_vkInstance);

        setupDebugMessenger();
        selectPhysicalDevice();
    }

    Runtime::~Runtime() {
        m_physicalDevice.reset();
        m_vkInstance.destroySurfaceKHR(m_surface);
        destroyDebugMessenger();
        m_vkInstance.destroy();
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

        m_vkInstance = vk::createInstance(createInfo);
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

        Ext::DebugUtils::createDebugUtilsMessengerEXT(m_vkInstance, debugCreateInfo, nullptr, &m_debugMessenger);
    }

    void Runtime::destroyDebugMessenger() const {
        Ext::DebugUtils::destroyDebugUtilsMessengerEXT(m_vkInstance, m_debugMessenger, nullptr);
    }

    void Runtime::selectPhysicalDevice() {
        m_surface = Window::CreateSurface(m_vkInstance);
        for (const auto devices = m_vkInstance.enumeratePhysicalDevices(); const auto &device : devices) {
            if (auto physicalDevice = std::make_unique<Core::PhysicalDevice>(device, m_surface); physicalDevice->isSuitable()) {
                m_physicalDevice = std::move(physicalDevice);
                return;
            }
        }
        if (!m_physicalDevice) {
            throw std::runtime_error("Failed to find a suitable physical device!");
        }
    }

    std::unique_ptr<Runtime> Runtime::m_instance = nullptr;

    void Runtime::Init() {
        m_instance = std::make_unique<Runtime>();
    }

    void Runtime::Destroy() {
        m_instance.reset();
    }
}