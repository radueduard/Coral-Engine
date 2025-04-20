//
// Created by radue on 4/18/2025.
//
export module core.commandBuffer;

import <vulkan/vulkan.hpp>;

import types;
import utils.wrapper;

import std;

namespace Coral::Core {
	export class CommandBuffer final : public Utils::Wrapper<vk::CommandBuffer> {
	public:
		CommandBuffer(u32 familyIndex, vk::CommandBuffer commandBuffer, const vk::CommandPool& parentCommandPool);

		~CommandBuffer() override;

		[[nodiscard]] const vk::CommandPool& ParentPool() const { return m_parentPool; }
		[[nodiscard]] const vk::Semaphore& SignalSemaphore() const { return m_signalSemaphore; }
		[[nodiscard]] const vk::Fence& Fence() const { return m_fence; }
		[[nodiscard]] u32 FamilyIndex() const { return m_familyIndex; }

	private:
		u32 m_familyIndex;
		const vk::CommandPool& m_parentPool;

		vk::Semaphore m_signalSemaphore;
		vk::Fence m_fence;
	};
}

