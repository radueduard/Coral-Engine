module core.commandBuffer;
import core.device;

namespace Coral::Core {
	CommandBuffer::CommandBuffer(const u32 familyIndex, const vk::CommandBuffer commandBuffer,
		const vk::CommandPool& parentCommandPool): m_familyIndex(familyIndex), m_parentPool(parentCommandPool) {
		m_handle = commandBuffer;
		m_signalSemaphore = GlobalDevice()->createSemaphore({});
		m_fence = GlobalDevice()->createFence(vk::FenceCreateInfo().setFlags(vk::FenceCreateFlagBits::eSignaled));
	}

	CommandBuffer::~CommandBuffer() {
		GlobalDevice().FreeCommandBuffer(*this);

		GlobalDevice()->destroySemaphore(m_signalSemaphore);
		GlobalDevice()->destroyFence(m_fence);
	}
}