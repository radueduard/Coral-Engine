//
// Created by radue on 10/13/2024.
//

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include "runtime.h"

#include <iostream>

#include "context.h"
#include "physicalDevice.h"
#include "window.h"

#include "extensions/debugUtils.h"


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

        Ext::DebugUtils::ImportFunctions(m_instance);
        // Ext::MeshShader::ImportFunctions(m_instance);

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