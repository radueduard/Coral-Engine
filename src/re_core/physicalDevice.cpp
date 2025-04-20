//
// Created by radue on 4/18/2025.
//

export module core.physicalDevice;

import <vulkan/vulkan.hpp>;

import utils.wrapper;
import types;

import std;

namespace Coral::Core {
	export class PhysicalDevice final : public Utils::Wrapper<vk::PhysicalDevice> {
		friend class Runtime;
	public:
		struct CreateInfo {
			vk::PhysicalDevice physicalDevice;
			vk::SurfaceKHR surface;
			std::unordered_set<vk::QueueFlagBits> requiredQueueFamilies;
			std::unordered_set<String> requiredExtensions;
			vk::PhysicalDeviceFeatures deviceFeatures;
		};

		explicit PhysicalDevice(const CreateInfo& createInfo) : m_surface(createInfo.surface)
		{
			m_requiredQueueFamilies = createInfo.requiredQueueFamilies;
			m_requiredExtensions = createInfo.requiredExtensions;
			m_deviceFeatures = createInfo.deviceFeatures;

			m_handle = createInfo.physicalDevice;
			m_properties = m_handle.getProperties();
			m_features = m_handle.getFeatures();
			m_memoryProperties = m_handle.getMemoryProperties();
			m_extensionProperties = m_handle.enumerateDeviceExtensionProperties();
			m_queueFamilyProperties = m_handle.getQueueFamilyProperties();

			QuerySurfaceCapabilities();
		}

		~PhysicalDevice() override = default;

		PhysicalDevice(const PhysicalDevice &) = delete;
		PhysicalDevice &operator=(const PhysicalDevice &) = delete;

		[[nodiscard]] bool IsSuitable() const {
			const bool hasRequiredQueueFamilies = HasRequiredQueueFamilies(m_requiredQueueFamilies);
			const bool hasRequiredExtensions = HasRequiredExtensions(m_requiredExtensions);
			const bool hasRequiredFeatures = HasRequiredFeatures(m_deviceFeatures);

			return hasRequiredQueueFamilies && hasRequiredExtensions && hasRequiredFeatures;
		}

		[[nodiscard]] const std::vector<vk::QueueFamilyProperties>& QueueFamilyProperties() const { return m_queueFamilyProperties; }

		[[nodiscard]] const vk::PhysicalDeviceProperties& Properties() const { return m_properties; }
		[[nodiscard]] const vk::SurfaceKHR& Surface() const { return m_surface; }
		[[nodiscard]] const vk::SurfaceCapabilitiesKHR& SurfaceCapabilities() const { return m_capabilities; }
		[[nodiscard]] const std::vector<vk::SurfaceFormatKHR>& SurfaceFormats() const { return m_formats; }
		[[nodiscard]] const std::vector<vk::PresentModeKHR>& SurfacePresentModes() const { return m_presentModes; }

		void QuerySurfaceCapabilities() {
			m_capabilities = m_handle.getSurfaceCapabilitiesKHR(m_surface);
			m_formats = m_handle.getSurfaceFormatsKHR(m_surface);
			m_presentModes = m_handle.getSurfacePresentModesKHR(m_surface);
		}

	private:
		[[nodiscard]] bool HasRequiredQueueFamilies(const std::unordered_set<vk::QueueFlagBits>& requiredQueueFamilies) const {
			VkBool32 presentSupported = false;
			std::unordered_set<vk::QueueFlagBits> supportedQueueFamilies {};

			for (const auto& queueFamily : m_queueFamilyProperties) {
				const auto queueFamilyIndex = static_cast<uint32_t>(&queueFamily - m_queueFamilyProperties.data());
				for (const auto& requiredQueueFamily : requiredQueueFamilies) {
					if (queueFamily.queueFlags & requiredQueueFamily) {
						supportedQueueFamilies.insert(requiredQueueFamily);
					}
				}
				presentSupported |= m_handle.getSurfaceSupportKHR(queueFamilyIndex, m_surface);
			}

			return requiredQueueFamilies == supportedQueueFamilies && presentSupported;
		}

		[[nodiscard]] bool HasRequiredExtensions(const std::unordered_set<String>& requiredExtensions) const {
			auto missingExtensions = requiredExtensions;
			for (const auto& availableExtension : m_extensionProperties) {
				String extensionName = availableExtension.extensionName;
				missingExtensions.erase(String(extensionName));
			}
			for (const auto& extension : missingExtensions) {
				std::cerr << "PhysicalDevice : Missing required extension " << extension << std::endl;
			}

			return missingExtensions.empty();
		}

		[[nodiscard]] bool HasRequiredFeatures(const vk::PhysicalDeviceFeatures& requiredFeatures) const {
			const auto requiredFeaturesPtr = reinterpret_cast<const vk::Bool32*>(&requiredFeatures);
			const auto deviceFeaturesPtr = reinterpret_cast<const vk::Bool32*>(&m_features);

			for (size_t i = 0; i < sizeof(vk::PhysicalDeviceFeatures) / sizeof(vk::Bool32); ++i) {
				if (requiredFeaturesPtr[i] && !deviceFeaturesPtr[i]) {
					return false;
				}
			}
			return true;
		}

		[[nodiscard]] bool IsSwapChainSupported() const { return !m_formats.empty() && !m_presentModes.empty(); }

		std::unordered_set<vk::QueueFlagBits> m_requiredQueueFamilies;
		std::unordered_set<String> m_requiredExtensions;
		vk::PhysicalDeviceFeatures m_deviceFeatures;

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