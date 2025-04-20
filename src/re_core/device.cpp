//
// Created by radue on 4/18/2025.
//
export module core.device;

import <vulkan/vulkan.hpp>;

import types;
import utils.wrapper;
import core.runtime;
import core.physicalDevice;
import core.queue;
import core.commandBuffer;
import std;

export std::thread::id mainThreadId = std::this_thread::get_id();

namespace Coral::Core {
	export class Device;

	Device* g_device = nullptr;
	export Device& GlobalDevice() {
		if (!g_device) {
			throw std::runtime_error("Device not initialized");
		}
		return *g_device;
	}

	class Device final : public Utils::Wrapper<vk::Device> {
	public:
		explicit Device() {
			const auto& physicalDevice = GlobalRuntime().PhysicalDevice();
			for (const auto& queueFamily : physicalDevice.QueueFamilyProperties()) {
				const auto queueFamilyIndex = static_cast<uint32_t>(&queueFamily - physicalDevice.QueueFamilyProperties().data());
				const bool canPresent = physicalDevice->getSurfaceSupportKHR(queueFamilyIndex, physicalDevice.Surface());

				m_queueFamilies.emplace_back(queueFamilyIndex, queueFamily, canPresent);
			}

			std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
			std::vector<std::vector<f32>> queuePriorities;
			for (const auto& queueFamily : m_queueFamilies) {
				queuePriorities.emplace_back(queueFamily.Properties().queueCount, 1.0f);
				const auto queueCreateInfo = vk::DeviceQueueCreateInfo()
					.setQueueFamilyIndex(queueFamily.Index())
					.setQueuePriorities(queuePriorities.back());
				queueCreateInfos.emplace_back(queueCreateInfo);
			}

			auto deviceMeshShaderFeatures = vk::PhysicalDeviceMeshShaderFeaturesEXT()
				.setTaskShader(false)
				.setMeshShader(true);

			const auto maintenance4Features = vk::PhysicalDeviceMaintenance4Features()
				.setMaintenance4(true)
				.setPNext(&deviceMeshShaderFeatures);

			const auto deviceCreateInfo = vk::DeviceCreateInfo()
				.setQueueCreateInfos(queueCreateInfos)
				.setPEnabledFeatures(&GlobalRuntime().DeviceFeatures())
				.setPNext(&maintenance4Features)
				.setPEnabledExtensionNames(GlobalRuntime().DeviceExtensions())
				.setPEnabledLayerNames(GlobalRuntime().DeviceLayers());

			m_handle = physicalDevice->createDevice(deviceCreateInfo);

			for (const auto& queueFamily : m_queueFamilies) {
				m_commandPools[queueFamily.Index()] = {};
			}
			CreateCommandPools(0);

			g_device = this;
		}
		~Device() override {
			g_device = nullptr;

			FreeCommandPools(0);
			m_handle.waitIdle();
			m_handle.destroy();
		}

		Device(const Device &) = delete;
		Device &operator=(const Device &) = delete;

		[[nodiscard]] std::unique_ptr<Queue> RequestQueue(const vk::QueueFlags type) {
			for (auto& queueFamily : m_queueFamilies) {
				if (!(queueFamily.Properties().queueFlags & type)) {
					continue;
				}
				try {
					return queueFamily.RequestQueue();
				} catch (const std::runtime_error&) {}
			}
			throw std::runtime_error("Queue::RequestQueue : Failed to find queue with requested flags");
		}

		[[nodiscard]] std::unique_ptr<Queue> RequestPresentQueue() {
			for (auto& queueFamily : m_queueFamilies) {
				try {
					return queueFamily.RequestPresentQueue();
				} catch (const std::runtime_error&) {}
			}
			throw std::runtime_error("Device::RequestPresentQueue: Failed to find suitable present queue!");
		}

		void CreateCommandPools(const u32 threadId) {
			for (const auto& queueFamily : m_queueFamilies) {
				const auto commandPoolCreateInfo = vk::CommandPoolCreateInfo()
					.setFlags(vk::CommandPoolCreateFlagBits::eTransient | vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
					.setQueueFamilyIndex(queueFamily.Index());
				m_commandPools[queueFamily.Index()].emplace(threadId, m_handle.createCommandPool(commandPoolCreateInfo));
			}
		}
		void FreeCommandPools(const u32 threadId) {
			for (const auto& queueFamily : m_queueFamilies) {
				m_handle.destroyCommandPool(m_commandPools[queueFamily.Index()][threadId]);
				m_commandPools[queueFamily.Index()].erase(threadId);
			}
		}

		[[nodiscard]] std::unique_ptr<CommandBuffer> RequestCommandBuffer(const u32 familyIndex, const u32 thread) const {
			const auto& commandPool = m_commandPools.at(familyIndex).at(thread);
			const auto commandBufferAllocInfo = vk::CommandBufferAllocateInfo()
				.setCommandPool(commandPool)
				.setLevel(vk::CommandBufferLevel::ePrimary)
				.setCommandBufferCount(1);
			const auto commandBuffers = m_handle.allocateCommandBuffers(commandBufferAllocInfo);
			return std::make_unique<CommandBuffer>(familyIndex, commandBuffers.front(), commandPool);
		}

		void FreeCommandBuffer(const CommandBuffer &commandBuffer) const {
			m_handle.freeCommandBuffers(commandBuffer.ParentPool(), *commandBuffer);
		}

		// TODO: Move this to PhysicalDevice
		[[nodiscard]] const PhysicalDevice& QuerySurfaceCapabilities() const {
			auto& physicalDevice = GlobalRuntime().PhysicalDevice();
			physicalDevice.QuerySurfaceCapabilities();
			return physicalDevice;
		}

		[[nodiscard]] std::optional<u32> FindMemoryType(const u32 typeFilter, const vk::MemoryPropertyFlags properties) const {
			const auto& physicalDevice = GlobalRuntime().PhysicalDevice();
			const auto memoryTypes = physicalDevice->getMemoryProperties().memoryTypes;
			for (usize i = 0; i < memoryTypes.size(); i++) {
				if (typeFilter & 1 << i && (memoryTypes[i].propertyFlags & properties) == properties)
					return i;
			}

			std::cerr << "Failed to find suitable memory type!" << std::endl;
			return std::nullopt;
		}

		void RunSingleTimeCommand(const std::function<void(const CommandBuffer&)> &command, const vk::QueueFlags requiredFlags,
		const vk::Fence fence = nullptr, vk::Semaphore waitSemaphore = nullptr, vk::Semaphore signalSemaphore = nullptr) {
			const auto threadId = std::this_thread::get_id();
			u32 thread = threadId._Get_underlying_id();
			if (threadId == mainThreadId) {
				thread = 0;
			}

			const auto queue = RequestQueue(requiredFlags);
			const auto commandBuffer = RequestCommandBuffer(queue->Family().Index(), thread);

			(*commandBuffer)->begin(vk::CommandBufferBeginInfo().setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
			command(*commandBuffer);
			(*commandBuffer)->end();

			const auto commandBuffers = std::array { **commandBuffer };
			const auto dstStageMask = std::vector<vk::PipelineStageFlags> { vk::PipelineStageFlagBits::eAllCommands };
			auto submitInfo = vk::SubmitInfo()
				.setCommandBuffers(commandBuffers);

			if (waitSemaphore != nullptr)
				submitInfo
					.setWaitSemaphores(waitSemaphore)
					.setWaitDstStageMask(dstStageMask);

			if (signalSemaphore != nullptr)
				submitInfo.setSignalSemaphores(signalSemaphore);

			try {
				(*queue)->submit(submitInfo, fence);
			} catch (const std::runtime_error& e) {
				std::cerr << e.what() << std::endl;
			}
			(*queue)->waitIdle();
		}

	private:
		std::vector<class Queue::Family> m_queueFamilies;
		std::unordered_map<uint32_t, std::unordered_map<uint32_t, vk::CommandPool>> m_commandPools;
	};

}