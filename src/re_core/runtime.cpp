//
// Created by radue on 4/18/2025.
//

export module core.runtime;

import <vulkan/vulkan.hpp>;

import types;
import core.window;
import core.physicalDevice;

import std;

namespace Coral::Core {
	export class Runtime;

	Runtime* g_runtime = nullptr;
	export const Runtime& GlobalRuntime() {
		if (!g_runtime) {
			throw std::runtime_error("Runtime not initialized");
		}
		return *g_runtime;
	}

	class Runtime {
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

		[[nodiscard]] const vk::Instance& Instance() const { return m_instance; }
		[[nodiscard]] const vk::SurfaceKHR& Surface() const { return m_surface; }
		[[nodiscard]] PhysicalDevice& PhysicalDevice() const { return *m_physicalDevice; }

		[[nodiscard]] const vk::PhysicalDeviceFeatures& DeviceFeatures() const { return m_deviceFeatures; }
		[[nodiscard]] const std::vector<const char*>& InstanceLayers() const { return m_instanceLayers; }
		[[nodiscard]] const std::vector<const char*>& InstanceExtensions() const { return m_instanceExtensions; }
		[[nodiscard]] const std::vector<const char*>& DeviceExtensions() const { return m_deviceExtensions; }
		[[nodiscard]] const std::vector<const char*>& DeviceLayers() const { return m_deviceLayers; }

	private:
		vk::Instance CreateInstance();
		void SelectPhysicalDevice();
		void SetupDebugMessenger();
		void DestroyDebugMessenger() const;

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

module : private;

import "extensions/debugUtils.h";
import "extensions/meshShader.h";

namespace Coral::Core {
	VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData)
	{
		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
		return VK_FALSE;
	}

	Runtime::Runtime(const CreateInfo& createInfo)
		: m_window(createInfo.window),
		  m_deviceFeatures(createInfo.deviceFeatures),
		  m_instanceLayers(createInfo.instanceLayers),
		  m_instanceExtensions(createInfo.instanceExtensions),
		  m_deviceExtensions(createInfo.deviceExtensions),
		  m_deviceLayers(createInfo.deviceLayers),
		  m_requiredQueueFamilies(createInfo.requiredQueueFamilies)
	{
		m_instance = CreateInstance();
		Ext::DebugUtils::ImportFunctions(m_instance);
		Ext::MeshShader::ImportFunctions(m_instance);

		SetupDebugMessenger();
		SelectPhysicalDevice();

		g_runtime = this;
	}

	Runtime::~Runtime() {
		g_runtime = nullptr;

		m_physicalDevice.reset();
		m_instance.destroySurfaceKHR(m_surface);
		DestroyDebugMessenger();
		m_instance.destroy();
	}

	vk::Instance Runtime::CreateInstance() {
		constexpr auto appInfo = vk::ApplicationInfo()
			.setPApplicationName("Vulkan Application")
			.setApplicationVersion(VK_MAKE_VERSION(1, 0, 0))
			.setPEngineName("Vulkan Graphics Engine")
			.setEngineVersion(VK_MAKE_VERSION(1, 0, 0))
			.setApiVersion(VK_API_VERSION_1_3);

		const auto windowExtensions = m_window.GetRequiredExtensions();
		m_instanceExtensions.insert(m_instanceExtensions.end(), windowExtensions.begin(), windowExtensions.end());

		const auto createInfo = vk::InstanceCreateInfo()
			.setPApplicationInfo(&appInfo)
			.setPEnabledExtensionNames(m_instanceExtensions)
			.setPEnabledLayerNames(m_instanceLayers);

		return vk::createInstance(createInfo);
	}

	void Runtime::SelectPhysicalDevice() {
		m_surface = m_window.CreateSurface(m_instance);
		m_physicalDevices = m_instance.enumeratePhysicalDevices();

		const std::unordered_set<String> requiredExtensions(m_deviceExtensions.begin(), m_deviceExtensions.end());
		for (const auto physicalDeviceCandidate : m_physicalDevices) {
			const PhysicalDevice::CreateInfo createInfo = {
				.physicalDevice = physicalDeviceCandidate,
				.surface = m_surface,
				.requiredQueueFamilies = m_requiredQueueFamilies,
				.requiredExtensions = requiredExtensions,
				.deviceFeatures = m_deviceFeatures,
			};

			if (auto physicalDevice = std::make_unique<Core::PhysicalDevice>(createInfo); physicalDevice->IsSuitable()) {
				if (physicalDevice->Properties().deviceType != vk::PhysicalDeviceType::eDiscreteGpu) {
					continue;
				}

				// Print physical device information
				std::cout << "Selected physical device: " << physicalDevice->Properties().deviceName << std::endl;
				std::cout << "Device type: " << vk::to_string(physicalDevice->Properties().deviceType) << std::endl;

				m_physicalDevice = std::move(physicalDevice);
				return;
			}
		}
		if (m_physicalDevice == nullptr) {
			throw std::runtime_error("Failed to find a suitable physical device!");
		}
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
			 .setPfnUserCallback(reinterpret_cast<vk::PFN_DebugUtilsMessengerCallbackEXT>(Coral::Core::DebugCallback));

		Ext::DebugUtils::createDebugUtilsMessengerEXT(m_instance, debugCreateInfo, nullptr, &m_debugMessenger);
	}

	void Runtime::DestroyDebugMessenger() const {
		Ext::DebugUtils::destroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
	}
}
