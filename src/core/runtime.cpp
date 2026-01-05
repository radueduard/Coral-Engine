//
// Created by radue on 10/13/2024.
//

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include "runtime.h"

#include <iostream>

#include "context.h"
#include "physicalDevice.h"
#include "window.h"


static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    const VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    const VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {

    // Choose color
    const char* color = "\x1b[37m";
    if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) {
        color = "\x1b[34m";
    } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        color = "\x1b[31m";
    } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        color = "\x1b[33m";
    }

    // Build severity label
    std::string severityLabel;
    if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        severityLabel = "ERROR";
    } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        severityLabel = "WARNING";
    } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
        severityLabel = "INFO";
    } else {
        severityLabel = "VERBOSE";
    }

    // Build type label(s)
    std::string typeLabel;
    bool first = true;
    if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT) {
        typeLabel += "GENERAL"; first = false;
    }
    if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) {
        if (!first) typeLabel += "|"; typeLabel += "VALIDATION"; first = false;
    }
    if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) {
        if (!first) typeLabel += "|"; typeLabel += "PERFORMANCE"; first = false;
    }
    // Print colored message with source location
    std::cout << color << "[" << severityLabel << "][" << typeLabel << "] "
              << pCallbackData->pMessage << "\x1b[0m" << std::endl;

    return messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT ? VK_TRUE : VK_FALSE;
}

namespace Coral::Core {
    Runtime::Runtime(const CreateInfo &createInfo) {
	    static bool firstInstance = true;
    	if (!firstInstance) {
    		throw std::runtime_error("Only one instance of Runtime is allowed!");
    	}
    	firstInstance = false;
    	Context::m_runtime = this;

    	m_deviceFeatures = createInfo.deviceFeatures;
    	m_deviceExtensions = createInfo.deviceExtensions;
#ifdef __APPLE__
		m_deviceExtensions.emplace_back("VK_KHR_portability_subset");
#endif

    	m_deviceLayers = createInfo.deviceLayers;
    	m_instanceExtensions = createInfo.instanceExtensions;

#ifdef __APPLE__
    	m_instanceExtensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
#endif

        m_instanceLayers = createInfo.instanceLayers;
        m_requiredQueueFamilies = createInfo.requiredQueueFamilies;

        CreateInstance();
    	VULKAN_HPP_DEFAULT_DISPATCHER.init(m_instance);

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
#ifdef __APPLE__
			.setFlags(vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR)
#endif
            .setPApplicationInfo(&appInfo)
            .setPEnabledExtensionNames(m_instanceExtensions)
            .setPEnabledLayerNames(m_instanceLayers);

        m_instance = vk::createInstance(createInfo);
    }

    void Runtime::SetupDebugMessenger() {
    	// const auto validationFeatures = std::vector {
   //  		vk::ValidationFeatureEnableEXT::eDebugPrintf,
   //  		vk::ValidationFeatureEnableEXT::eBestPractices
   //  	};
	  //
   //  	const auto validationCreateInfo = vk::ValidationFeaturesEXT()
			// .setEnabledValidationFeatures(validationFeatures);

        const auto debugCreateInfo = vk::DebugUtilsMessengerCreateInfoEXT()
    		// .setPNext(&validationCreateInfo)
            .setMessageSeverity(
                vk::DebugUtilsMessageSeverityFlagBitsEXT::eError
                | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning
                | vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose
                // | vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo
            )
            .setMessageType(
                vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation)
            .setPfnUserCallback(reinterpret_cast<vk::PFN_DebugUtilsMessengerCallbackEXT>(debugCallback));

    	m_debugMessenger = m_instance.createDebugUtilsMessengerEXT(debugCreateInfo);
    }

    void Runtime::destroyDebugMessenger() const {
    	m_instance.destroyDebugUtilsMessengerEXT(m_debugMessenger);
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
                // if (physicalDevice->m_properties.deviceType != vk::PhysicalDeviceType::eDiscreteGpu) {
                //     continue;
                // }

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