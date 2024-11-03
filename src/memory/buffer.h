//
// Created by radue on 10/19/2024.
//

#pragma once

#include "core/device.h"
#include <iostream>

namespace Memory {
    template<typename T>
    class Buffer {
    public:
        Buffer(const Core::Device &device,
            uint32_t instanceCount,
            vk::BufferUsageFlags usage,
            vk::MemoryPropertyFlags properties,
            vk::DeviceSize minOffsetAlignment = 0);
        ~Buffer();

        Buffer(const Buffer &) = delete;
        Buffer &operator=(const Buffer &) = delete;

        /**
         * @brief Maps the buffer memory window
         *
         * @param instanceCount The size of the buffer to map (in instances)
         * @param offset The offset in the buffer to start mapping at (in instances)
         */
        void Map(vk::DeviceSize instanceCount = vk::WholeSize, vk::DeviceSize offset = 0);

        /**
         * @brief Unmaps the buffer memory window
         */
        void Unmap();

        /**
         * @brief Writes data to the buffer
         *
         * @param data A raw pointer to the data to be written to the buffer
         * @param instanceCount The number of instances to write. If vk::WholeSize, the entire buffer will be written
         * @param offset The offset in the buffer to start writing at (in instances). If instanceCount is vk::WholeSize, this is ignored
         */
        void Write(const T *data, vk::DeviceSize instanceCount = vk::WholeSize, vk::DeviceSize offset = 0);

        /**
         * @brief Flushes the memory to make it visible to the GPU. This is done for all the mapped memory
         */
        void Flush(vk::DeviceSize instanceCount = vk::WholeSize, vk::DeviceSize offset = 0);

        /**
         * @brief Returns a descriptor buffer info for the buffer
         *
         * @param instanceCount The size of the buffer to use in the descriptor
         * @param offset The offset in the buffer to start at (in instances)
         * @return The descriptor buffer info for the buffer
         */
        [[nodiscard]] std::optional<vk::DescriptorBufferInfo> DescriptorInfo(vk::DeviceSize instanceCount = vk::WholeSize, vk::DeviceSize offset = 0) const;

        /**
         * @brief Invalidates the buffer memory that is mapped
         */
        void Invalidate(vk::DeviceSize instanceCount = vk::WholeSize, vk::DeviceSize offset = 0);

        /**
         * @brief Reads data from the buffer
         *
         * @param index The index in the buffer to start reading at
         * @return The data read from the buffer
         */
        std::optional<T> ReadAt(uint32_t index) const;

        /**
         * @brief Writes data to a specific index in the buffer
         *
         * @param index The index in the buffer to write the data to
         * @param data A raw pointer to the data to be written to the buffer
         */
        void WriteAt(uint32_t index, const T& data);

        /**
         * @brief Flushes the memory at a specific index in the buffer
         *
         * @param index The index in the buffer to flush
         */
        void FlushAt(uint32_t index);

        /**
         * @brief Returns a descriptor buffer info for a specific index in the buffer
         *
         * @param index The index in the buffer to get the descriptor buffer info for
         * @return The descriptor buffer info for the buffer at the specified index
         */
        [[nodiscard]] std::optional<vk::DescriptorBufferInfo> DescriptorInfoAt(uint32_t index) const;

        /**
         * @brief Invalidates the buffer memory at a specific index
         *
         * @param index The index in the buffer to invalidate
         */
        void InvalidateAt(uint32_t index);

        /**
         *
         * @param srcBuffer The buffer to copy from
         * @param instanceCount The number of instances to copy. If vk::WholeSize, the entire buffer will be copied
         * @param srcOffset The offset in the source buffer to start copying from (in instances)
         * @param dstOffset The offset in the destination buffer to start copying to (in instances)
         *
         * @remark If the instanceCount is vk::WholeSize, the srcOffset and dstOffset are ignored
         */
        void CopyBuffer(const Buffer<T> &srcBuffer, vk::DeviceSize instanceCount = vk::WholeSize, vk::DeviceSize srcOffset = 0, vk::DeviceSize dstOffset = 0) const;

        [[nodiscard]]

        vk::Buffer operator *() const { return m_buffer; }
        T *Data() const { return reinterpret_cast<T *>(m_mapped); }
        [[nodiscard]] uint32_t InstanceCount() const { return m_instanceCount; }
        [[nodiscard]] vk::DeviceSize AlignmentSize() const { return m_alignmentSize; }
        [[nodiscard]] vk::DeviceSize Size() const { return m_instanceCount * m_alignmentSize; }
        [[nodiscard]] vk::BufferUsageFlags UsageFlags() const { return m_usageFlags; }
        [[nodiscard]] vk::MemoryPropertyFlags MemoryPropertyFlags() const { return m_memoryPropertyFlags; }


    private:
        const Core::Device &m_device;
        vk::Buffer m_buffer;
        vk::DeviceMemory m_memory;

        uint32_t m_instanceCount;
        T* m_mapped = nullptr;
        vk::MappedMemoryRange m_mappedRange;

        vk::DeviceSize m_alignmentSize;
        vk::BufferUsageFlags m_usageFlags;
        vk::MemoryPropertyFlags m_memoryPropertyFlags;
    };
}

static vk::DeviceSize GetAlignment(const vk::DeviceSize size, const vk::DeviceSize alignment) {
    if (alignment > 0) {
        return (size + alignment - 1) & ~(alignment - 1);
    }
    return size;
}

namespace Memory {
    template<typename T>
    Buffer<T>::Buffer(
        const Core::Device &device,
        const uint32_t instanceCount,
        const vk::BufferUsageFlags usage,
        const vk::MemoryPropertyFlags properties,
        const vk::DeviceSize minOffsetAlignment)
        : m_device(device), m_instanceCount(instanceCount), m_usageFlags(usage), m_memoryPropertyFlags(properties)
    {
        m_alignmentSize = GetAlignment(sizeof(T), minOffsetAlignment);
        const auto bufferSize = m_alignmentSize * m_instanceCount;

        const auto bufferInfo = vk::BufferCreateInfo()
            .setSize(bufferSize)
            .setUsage(usage)
            .setSharingMode(vk::SharingMode::eExclusive);

        m_buffer = (*m_device).createBuffer(bufferInfo);

        const auto memRequirements = (*m_device).getBufferMemoryRequirements(m_buffer);
        const auto memoryTypeIndex = m_device.FindMemoryType(memRequirements.memoryTypeBits, properties);
        if (!memoryTypeIndex.has_value()) {
            std::cerr << "Buffer : Failed to find suitable memory type" << std::endl;
        }

        const auto allocInfo = vk::MemoryAllocateInfo()
            .setAllocationSize(memRequirements.size)
            .setMemoryTypeIndex(memoryTypeIndex.value());

        m_memory = (*m_device).allocateMemory(allocInfo);
        (*m_device).bindBufferMemory(m_buffer, m_memory, 0);
    }

    template<typename T>
    Buffer<T>::~Buffer()
    {
        Unmap();
        (*m_device).destroyBuffer(m_buffer);
        (*m_device).freeMemory(m_memory);
    }

    template<typename T>
    void Buffer<T>::Map(vk::DeviceSize instanceCount, vk::DeviceSize offset) {
        if (!(m_buffer && m_memory)) {
            std::cerr << "Buffer::Map : Buffer or memory not ready" << std::endl;
            return;
        }

        if (!(m_memoryPropertyFlags & vk::MemoryPropertyFlagBits::eHostVisible)) {
            std::cerr << "Buffer::Map : Memory not host visible" << std::endl;
            return;
        }

        if (instanceCount == vk::WholeSize) {
            instanceCount = m_instanceCount;
            offset = 0;
        }
        m_mapped = static_cast<T*>((*m_device).mapMemory(
            m_memory,
            offset * m_alignmentSize,
            instanceCount * m_alignmentSize,
            vk::MemoryMapFlags()));

        m_mappedRange = vk::MappedMemoryRange()
            .setMemory(m_memory)
            .setSize(instanceCount * m_alignmentSize)
            .setOffset(offset * m_alignmentSize);
    }

    template<typename T>
    void Buffer<T>::Unmap() {
        if (m_mapped) {
            (*m_device).unmapMemory(m_memory);
            m_mapped = nullptr;
        }
    }

    template<typename T>
    void Buffer<T>::Write(const T *data, vk::DeviceSize instanceCount, vk::DeviceSize offset) {
        if (!m_mapped) {
            std::cerr << "Buffer::Write : Buffer not mapped" << std::endl;
            return;
        }

        if (instanceCount == vk::WholeSize) {
            instanceCount = m_instanceCount;
            offset = 0;
        }

        if (instanceCount + offset > m_instanceCount) {
            std::cerr << "Buffer::Write : Instance count out of bounds" << std::endl;
            return;
        }

        auto memOffset = m_mapped + offset;
        std::memcpy(memOffset, data, instanceCount * m_alignmentSize);
    }

    template<typename T>
    void Buffer<T>::Flush(const vk::DeviceSize instanceCount, const vk::DeviceSize offset) {
        if (!m_mapped) {
            std::cerr << "Buffer::Flush : Buffer not mapped" << std::endl;
            return;
        }

        if (instanceCount == vk::WholeSize) {
            (*m_device).flushMappedMemoryRanges(m_mappedRange);
            return;
        }

        const auto range = vk::MappedMemoryRange()
            .setMemory(m_memory)
            .setSize(instanceCount * m_alignmentSize)
            .setOffset(offset * m_alignmentSize);
        (*m_device).flushMappedMemoryRanges(range);
    }

    template<typename T>
    std::optional<vk::DescriptorBufferInfo> Buffer<T>::DescriptorInfo(vk::DeviceSize instanceCount, vk::DeviceSize offset) const {
        if (instanceCount == vk::WholeSize) {
            instanceCount = m_instanceCount;
            offset = 0;
        }

        if (instanceCount + offset > m_instanceCount) {
            std::cerr << "Buffer::DescriptorInfo : Instance count out of bounds" << std::endl;
            return std::nullopt;
        }

        return vk::DescriptorBufferInfo()
            .setBuffer(m_buffer)
            .setOffset(offset * m_alignmentSize)
            .setRange(instanceCount * m_alignmentSize);
    }

    template<typename T>
    void Buffer<T>::Invalidate(const vk::DeviceSize instanceCount, const vk::DeviceSize offset) {
        if (!m_mapped) {
            std::cerr << "Buffer::Invalidate : Buffer not mapped" << std::endl;
            return;
        }
        if (instanceCount == vk::WholeSize) {
            (*m_device).invalidateMappedMemoryRanges(m_mappedRange);
            return;
        }
        const auto range = vk::MappedMemoryRange()
            .setMemory(m_memory)
            .setSize(instanceCount * m_alignmentSize)
            .setOffset(offset * m_alignmentSize);
        (*m_device).invalidateMappedMemoryRanges(range);
    }

    template<typename T>
    std::optional<T> Buffer<T>::ReadAt(const uint32_t index) const {
        if (index >= m_instanceCount) {
            std::cerr << "Buffer::WriteAt : Index out of bounds" << std::endl;
            return std::nullopt;
        }
        return m_mapped[index];
    }

    template<typename T>
    void Buffer<T>::WriteAt(const uint32_t index, const T &data) {
        if (index >= m_instanceCount) {
            std::cerr << "Buffer::WriteAt : Index out of bounds" << std::endl;
            return;
        }
        Write(&data, 1, index);
    }

    template<typename T>
    void Buffer<T>::FlushAt(const uint32_t index) {
        if (index >= m_instanceCount) {
            std::cerr << "Buffer::FlushAt : Index out of bounds" << std::endl;
            return;
        }

        Flush(1, index);
    }

    template<typename T>
    std::optional<vk::DescriptorBufferInfo> Buffer<T>::DescriptorInfoAt(const uint32_t index) const {
        if (index >= m_instanceCount) {
            std::cerr << "Buffer::DescriptorInfoAt : Index out of bounds" << std::endl;
            return std::nullopt;
        }

        return DescriptorInfo(1, index);
    }

    template<typename T>
    void Buffer<T>::InvalidateAt(const uint32_t index) {
        if (index >= m_instanceCount) {
            std::cerr << "Buffer::InvalidateAt : Index out of bounds" << std::endl;
            return;
        }

        Invalidate(1, index);
    }

    template<typename T>
    void Buffer<T>::CopyBuffer(const Buffer &srcBuffer, vk::DeviceSize instanceCount, vk::DeviceSize srcOffset, vk::DeviceSize dstOffset) const {
        if (instanceCount == vk::WholeSize) {
            instanceCount = m_instanceCount;
            srcOffset = 0;
            dstOffset = 0;
        }

        const auto commandBuffer = m_device.BeginSingleTimeCommands(Core::Queue::Type::Transfer);

        const auto copyRegion = vk::ArrayProxy {
            vk::BufferCopy()
                .setSrcOffset(srcOffset * m_alignmentSize)
                .setDstOffset(dstOffset * m_alignmentSize)
                .setSize(instanceCount * m_alignmentSize)
        };

        commandBuffer.copyBuffer(*srcBuffer, m_buffer, copyRegion);
        m_device.EndSingleTimeCommands(commandBuffer, Core::Queue::Type::Transfer);
    }
}