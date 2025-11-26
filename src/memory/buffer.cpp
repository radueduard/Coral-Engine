//
// Created by radue on 11/24/2025.
//

#include "buffer.h"

Coral::Memory::Buffer::Builder::Builder() { m_name = to_string(boost::uuids::random_generator()()); }
Coral::Memory::Buffer::Builder::~Builder() = default;
Coral::Memory::Buffer::Builder& Coral::Memory::Buffer::Builder::InstanceSize(const u32 instanceSize) {
	m_instanceSize = instanceSize;
	return *this;
}
Coral::Memory::Buffer::Builder& Coral::Memory::Buffer::Builder::InstanceCount(const u32 instanceCount) {
	m_instanceCount = instanceCount;
	return *this;
}
Coral::Memory::Buffer::Builder& Coral::Memory::Buffer::Builder::UsageFlags(const vk::BufferUsageFlagBits usageFlag) {
	m_usageFlagSet.insert(usageFlag);
	return *this;
}
Coral::Memory::Buffer::Builder&
Coral::Memory::Buffer::Builder::MemoryProperty(const vk::MemoryPropertyFlagBits memoryPropertyFlag) {
	m_memoryPropertyFlagSet.insert(memoryPropertyFlag);
	return *this;
}
Coral::Memory::Buffer::Builder& Coral::Memory::Buffer::Builder::DeviceAlignment(const vk::DeviceSize deviceAlignment) {
	m_deviceAlignment = deviceAlignment;
	return *this;
}
std::unique_ptr<Coral::Memory::Buffer> Coral::Memory::Buffer::Builder::Build() {
	return std::make_unique<Buffer>(*this);
}
Coral::Memory::Buffer::Buffer(const Builder& builder) : m_instanceCount(builder.m_instanceCount) {
	for (const auto& flag : builder.m_usageFlagSet) {
		m_usageFlags |= flag;
	}
	for (const auto& flag : builder.m_memoryPropertyFlagSet) {
		m_memoryPropertyFlags |= flag;
	}

	m_alignmentSize = GetAlignment(builder.m_instanceSize, builder.m_deviceAlignment);
	const auto bufferSize = m_alignmentSize * m_instanceCount;

	const auto bufferInfo =
		vk::BufferCreateInfo().setSize(bufferSize).setUsage(m_usageFlags).setSharingMode(vk::SharingMode::eExclusive);

	m_handle = Context::Device()->createBuffer(bufferInfo);

	const auto memRequirements = Context::Device()->getBufferMemoryRequirements(m_handle);
	const auto memoryTypeIndex =
		Context::Device().FindMemoryType(memRequirements.memoryTypeBits, m_memoryPropertyFlags);

	if (!memoryTypeIndex.has_value()) {
		std::cerr << "Buffer : Failed to find suitable memory type" << std::endl;
	}

	const auto allocInfo =
		vk::MemoryAllocateInfo().setAllocationSize(memRequirements.size).setMemoryTypeIndex(memoryTypeIndex.value());

	m_memory = Context::Device()->allocateMemory(allocInfo);
	Context::Device()->bindBufferMemory(m_handle, m_memory, 0);
}
Coral::Memory::Buffer::~Buffer() {
	Unmap();
	Context::Device()->destroyBuffer(m_handle);
	Context::Device()->freeMemory(m_memory);
}

void Coral::Memory::Buffer::Flush(const vk::DeviceSize instanceCount, const vk::DeviceSize offset) const {
	if (!m_mapped) {
		std::cerr << "Buffer::Flush : Buffer not mapped" << std::endl;
		return;
	}

	if (instanceCount == vk::WholeSize) {
		Context::Device()->flushMappedMemoryRanges(m_mappedRange);
		return;
	}

	const auto range = vk::MappedMemoryRange()
						   .setMemory(m_memory)
						   .setSize(instanceCount * m_alignmentSize)
						   .setOffset(offset * m_alignmentSize);
	Context::Device()->flushMappedMemoryRanges(range);
}
vk::DescriptorBufferInfo Coral::Memory::Buffer::DescriptorInfo(vk::DeviceSize instanceCount,
															   vk::DeviceSize offset) const {
	if (instanceCount == vk::WholeSize) {
		instanceCount = m_instanceCount;
		offset = 0;
	}

	if (instanceCount + offset > m_instanceCount) {
		throw std::runtime_error("Buffer::DescriptorInfo : Instance count out of bounds");
	}

	return vk::DescriptorBufferInfo()
		.setBuffer(m_handle)
		.setOffset(offset * m_alignmentSize)
		.setRange(instanceCount * m_alignmentSize);
}
void Coral::Memory::Buffer::Invalidate(const vk::DeviceSize instanceCount, const vk::DeviceSize offset) const {
	if (!m_mapped) {
		std::cerr << "Buffer::Invalidate : Buffer not mapped" << std::endl;
		return;
	}

	if (instanceCount == vk::WholeSize) {
		Context::Device()->invalidateMappedMemoryRanges(m_mappedRange);
		return;
	}
	const auto range = vk::MappedMemoryRange()
						   .setMemory(m_memory)
						   .setSize(instanceCount * m_alignmentSize)
						   .setOffset(offset * m_alignmentSize);
	Context::Device()->invalidateMappedMemoryRanges(range);
}

void Coral::Memory::Buffer::FlushAt(const u32 index) const {
	if (!m_mapped) {
		std::cerr << "Buffer::FlushAt : Buffer not mapped" << std::endl;
		return;
	}

	if (index >= m_instanceCount) {
		std::cerr << "Buffer::FlushAt : Index out of bounds" << std::endl;
		return;
	}

	Flush(1, index);
}
vk::DescriptorBufferInfo Coral::Memory::Buffer::DescriptorInfoAt(const u32 index) const {
	if (index >= m_instanceCount) {
		throw std::runtime_error("Buffer::DescriptorInfoAt : Index out of bounds");
	}
	return DescriptorInfo(1, index);
}
void Coral::Memory::Buffer::InvalidateAt(const u32 index) const {
	if (index >= m_instanceCount) {
		std::cerr << "Buffer::InvalidateAt : Index out of bounds" << std::endl;
		return;
	}
	Invalidate(1, index);
}
void Coral::Memory::Buffer::CopyBuffer(const std::unique_ptr<Buffer>& srcBuffer, const vk::DeviceSize instanceCount,
									   vk::DeviceSize srcOffset, vk::DeviceSize dstOffset) const {
	if (m_alignmentSize != srcBuffer->m_alignmentSize) {
		std::cerr << "Buffer::CopyBuffer : Alignment sizes do not match" << std::endl;
		return;
	}

	auto size = instanceCount;
	if (instanceCount != vk::WholeSize) {
		size *= m_alignmentSize;
		srcOffset *= m_alignmentSize;
		dstOffset *= m_alignmentSize;
	}
	else {
		size = std::min(m_instanceCount, srcBuffer->m_instanceCount) * m_alignmentSize;
		srcOffset = 0;
		dstOffset = 0;
	}

	Context::Device().RunSingleTimeCommand(
		[this, srcOffset, dstOffset, &srcBuffer, size](const Core::CommandBuffer& commandBuffer) {
			const auto copyRegion = vk::BufferCopy().setSrcOffset(srcOffset).setDstOffset(dstOffset).setSize(size);
			commandBuffer->copyBuffer(**srcBuffer, m_handle, 1, &copyRegion);
		},
		vk::QueueFlagBits::eTransfer);
}
uint32_t Coral::Memory::Buffer::InstanceCount() const { return m_instanceCount; }
vk::DeviceSize Coral::Memory::Buffer::AlignmentSize() const { return m_alignmentSize; }
vk::DeviceSize Coral::Memory::Buffer::Size() const { return m_instanceCount * m_alignmentSize; }
vk::BufferUsageFlags Coral::Memory::Buffer::UsageFlags() const { return m_usageFlags; }
vk::MemoryPropertyFlags Coral::Memory::Buffer::MemoryPropertyFlags() const { return m_memoryPropertyFlags; }
