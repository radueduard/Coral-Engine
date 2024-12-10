//
// Created by radue on 12/6/2024.
//

#include "buffer.h"

#include <iostream>

Memory::Buffer::Buffer(
    const vk::DeviceSize instanceSize,
    const uint32_t instanceCount,
    const vk::BufferUsageFlags usage,
    const vk::MemoryPropertyFlags properties,
    const vk::DeviceSize minOffsetAlignment)
    : m_instanceSize(instanceSize), m_instanceCount(instanceCount), m_usageFlags(usage), m_memoryPropertyFlags(properties)
{
    const auto& device = Core::Device::Get();
    m_alignmentSize = GetAlignment(instanceSize, minOffsetAlignment);
    const auto bufferSize = m_alignmentSize * m_instanceCount;

    const auto bufferInfo = vk::BufferCreateInfo()
        .setSize(bufferSize)
        .setUsage(usage)
        .setSharingMode(vk::SharingMode::eExclusive);

    m_buffer = (*device).createBuffer(bufferInfo);

    const auto memRequirements = (*device).getBufferMemoryRequirements(m_buffer);
    const auto memoryTypeIndex = device.FindMemoryType(memRequirements.memoryTypeBits, properties);

    if (!memoryTypeIndex.has_value()) {
        std::cerr << "Buffer : Failed to find suitable memory type" << std::endl;
    }

    const auto allocInfo = vk::MemoryAllocateInfo()
        .setAllocationSize(memRequirements.size)
        .setMemoryTypeIndex(memoryTypeIndex.value());

    m_memory = (*device).allocateMemory(allocInfo);
    (*device).bindBufferMemory(m_buffer, m_memory, 0);
}

Memory::Buffer::~Buffer() {
    Unmap();
    const auto& device = Core::Device::Get();
    (*device).destroyBuffer(m_buffer);
    (*device).freeMemory(m_memory);
}

void Memory::Buffer::Unmap() {
    if (m_mapped) {
        (*Core::Device::Get()).unmapMemory(m_memory);
        m_mapped = nullptr;
    }
}



void Memory::Buffer::Flush(const vk::DeviceSize instanceCount, const vk::DeviceSize offset) const {
    if (!m_mapped) {
        std::cerr << "Buffer::Flush : Buffer not mapped" << std::endl;
        return;
    }

    if (instanceCount == vk::WholeSize) {
        (*Core::Device::Get()).flushMappedMemoryRanges(m_mappedRange);
        return;
    }

    const auto range = vk::MappedMemoryRange()
        .setMemory(m_memory)
        .setSize(instanceCount * m_alignmentSize)
        .setOffset(offset * m_alignmentSize);
    (*Core::Device::Get()).flushMappedMemoryRanges(range);
}

vk::DescriptorBufferInfo Memory::Buffer::DescriptorInfo(vk::DeviceSize instanceCount, vk::DeviceSize offset) const {
    if (instanceCount == vk::WholeSize) {
        instanceCount = m_instanceCount;
        offset = 0;
    }

    if (instanceCount + offset > m_instanceCount) {
        throw std::runtime_error("Buffer::DescriptorInfo : Instance count out of bounds");
    }

    return vk::DescriptorBufferInfo()
        .setBuffer(m_buffer)
        .setOffset(offset * m_alignmentSize)
        .setRange(instanceCount * m_alignmentSize);
}

void Memory::Buffer::Invalidate(const vk::DeviceSize instanceCount, const vk::DeviceSize offset) const {
    if (!m_mapped) {
        std::cerr << "Buffer::Invalidate : Buffer not mapped" << std::endl;
        return;
    }

    if (instanceCount == vk::WholeSize) {
        (*Core::Device::Get()).invalidateMappedMemoryRanges(m_mappedRange);
        return;
    }
    const auto range = vk::MappedMemoryRange()
        .setMemory(m_memory)
        .setSize(instanceCount * m_alignmentSize)
        .setOffset(offset * m_alignmentSize);
    (*Core::Device::Get()).invalidateMappedMemoryRanges(range);
}

void Memory::Buffer::FlushAt(const uint32_t index) {
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


vk::DescriptorBufferInfo Memory::Buffer::DescriptorInfoAt(const uint32_t index) const {
    if (index >= m_instanceCount) {
        throw std::runtime_error("Buffer::DescriptorInfoAt : Index out of bounds");
    }
    return DescriptorInfo(1, index);
}

void Memory::Buffer::InvalidateAt(const uint32_t index) const {
    if (index >= m_instanceCount) {
        std::cerr << "Buffer::InvalidateAt : Index out of bounds" << std::endl;
        return;
    }
    Invalidate(1, index);
}

void Memory::Buffer::CopyBuffer(const std::unique_ptr<Buffer> &srcBuffer, const vk::DeviceSize instanceCount, vk::DeviceSize srcOffset, vk::DeviceSize dstOffset) const {
    if (m_alignmentSize != srcBuffer->m_alignmentSize) {
        std::cerr << "Buffer::CopyBuffer : Alignment sizes do not match" << std::endl;
        return;
    }

    auto size = instanceCount;
    if (instanceCount != vk::WholeSize) {
        size *= m_alignmentSize;
        srcOffset *= m_alignmentSize;
        dstOffset *= m_alignmentSize;
    } else {
        size = std::min(m_instanceCount, srcBuffer->m_instanceCount) * m_alignmentSize;
        srcOffset = 0;
        dstOffset = 0;
    }

    Core::Device::Get().RunSingleTimeCommand([this, srcOffset, dstOffset, &srcBuffer, size](const vk::CommandBuffer &commandBuffer) {
        const auto copyRegion = vk::BufferCopy()
            .setSrcOffset(srcOffset)
            .setDstOffset(dstOffset)
            .setSize(size);
        commandBuffer.copyBuffer(**srcBuffer, m_buffer, 1, &copyRegion);
    }, vk::QueueFlagBits::eTransfer);
}