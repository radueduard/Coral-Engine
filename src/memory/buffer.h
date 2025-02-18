//
// Created by radue on 10/19/2024.
//

#pragma once

#include <vulkan/vulkan.hpp>

#include "core/device.h"

namespace Memory {
    class Buffer {
    public:
        Buffer(
            const Core::Device& device,
            vk::DeviceSize instanceSize,
            uint32_t instanceCount,
            vk::BufferUsageFlags usage,
            vk::MemoryPropertyFlags properties,
            vk::DeviceSize minOffsetAlignment = 0);
        ~Buffer();

        Buffer(const Buffer &) = delete;
        Buffer &operator=(const Buffer &) = delete;

        template <typename T>
        std::span<T> Map(vk::DeviceSize instanceCount = vk::WholeSize, vk::DeviceSize offset = 0);
        void Unmap();

        template <typename T>
        std::span<T> Read(vk::DeviceSize instanceCount = vk::WholeSize, vk::DeviceSize offset = 0);
        template <typename T>
        void Write(const std::span<T>&, vk::DeviceSize offset = 0);
        void Flush(vk::DeviceSize instanceCount = vk::WholeSize, vk::DeviceSize offset = 0) const;
        [[nodiscard]] vk::DescriptorBufferInfo DescriptorInfo(vk::DeviceSize instanceCount = vk::WholeSize, vk::DeviceSize offset = 0) const;
        void Invalidate(vk::DeviceSize instanceCount = vk::WholeSize, vk::DeviceSize offset = 0) const;

        template <typename T>
        [[nodiscard]] T ReadAt(uint32_t index) const;
        template <typename T>
        void WriteAt(uint32_t index, const T& data);
        void FlushAt(uint32_t index);
        [[nodiscard]] vk::DescriptorBufferInfo DescriptorInfoAt(uint32_t index) const;
        void InvalidateAt(uint32_t index) const;

        void CopyBuffer(const std::unique_ptr<Buffer> &srcBuffer, vk::DeviceSize instanceCount = vk::WholeSize, vk::DeviceSize srcOffset = 0, vk::DeviceSize dstOffset = 0) const;

        [[nodiscard]] vk::Buffer operator *() const { return m_buffer; }
        [[nodiscard]] vk::DeviceSize InstanceSize() const { return m_instanceSize; }
        [[nodiscard]] uint32_t InstanceCount() const { return m_instanceCount; }
        [[nodiscard]] vk::DeviceSize AlignmentSize() const { return m_alignmentSize; }
        [[nodiscard]] vk::DeviceSize Size() const { return m_instanceCount * m_alignmentSize; }
        [[nodiscard]] vk::BufferUsageFlags UsageFlags() const { return m_usageFlags; }
        [[nodiscard]] vk::MemoryPropertyFlags MemoryPropertyFlags() const { return m_memoryPropertyFlags; }


    private:
        const Core::Device& m_device;

        vk::Buffer m_buffer;
        vk::DeviceMemory m_memory;

        const vk::DeviceSize m_instanceSize;
        uint32_t m_instanceCount;
        void* m_mapped = nullptr;
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

template <typename T>
std::span<T> Memory::Buffer::Map(vk::DeviceSize instanceCount, const vk::DeviceSize offset) {
    static_assert(std::is_trivially_copyable_v<T>, "Type must be trivially copyable");
    static_assert(std::is_standard_layout_v<T>, "Type must be standard layout");

    if (!(m_buffer && m_memory)) {
        throw std::runtime_error("Buffer::Map : Buffer or memory not ready");
    }

    if (!(m_memoryPropertyFlags & vk::MemoryPropertyFlagBits::eHostVisible)) {
        throw std::runtime_error("Buffer::Map : Memory not host visible");
    }

    if (instanceCount != vk::WholeSize) {
        instanceCount *= m_alignmentSize;
    }

    m_mapped = m_device.Handle().mapMemory(
        m_memory,
        offset * m_alignmentSize,
        instanceCount,
        vk::MemoryMapFlags());

    m_mappedRange = vk::MappedMemoryRange()
        .setMemory(m_memory)
        .setSize(instanceCount)
        .setOffset(offset * m_alignmentSize);

    return std::span<T>(static_cast<T *>(m_mapped), instanceCount);
}

template<typename T>
std::span<T> Memory::Buffer::Read(vk::DeviceSize instanceCount, vk::DeviceSize offset) {
    static_assert(std::is_trivially_copyable_v<T>, "Type must be trivially copyable");
    static_assert(std::is_standard_layout_v<T>, "Type must be standard layout");

    if (!m_mapped) {
        throw std::runtime_error("Buffer::Read : Buffer not mapped");
    }

    if (sizeof(T) != m_instanceSize) {
        throw std::runtime_error("Buffer::Map : Size of type does not match instance size");
    }

    if (instanceCount == vk::WholeSize) {
        instanceCount = m_instanceCount;
        offset = 0;
    }

    if (instanceCount + offset > m_instanceCount) {
        throw std::runtime_error("Buffer::Read : Instance count out of bounds");
    }

    return std::span<T>(static_cast<T *>(m_mapped + offset * m_alignmentSize), instanceCount);
}

template<typename T>
void Memory::Buffer::Write(const std::span<T>& data, vk::DeviceSize offset) {
    static_assert(std::is_trivially_copyable_v<T>, "Type must be trivially copyable");
    static_assert(std::is_standard_layout_v<T>, "Type must be standard layout");

    if (!m_mapped) {
        throw std::runtime_error("Buffer::Write : Buffer not mapped");
    }

    if (sizeof(T) != m_instanceSize) {
        throw std::runtime_error("Buffer::Write : Size of type does not match instance size");
    }

    if (data.size() + offset > m_instanceCount) {
        throw std::runtime_error("Buffer::Write : Data size exceeds buffer size");
    }

    void* mapped = static_cast<uint8_t*>(m_mapped) + offset * m_alignmentSize;
    std::memcpy(mapped, data.data(), data.size_bytes());
}

template<typename T>
T Memory::Buffer::ReadAt(const uint32_t index) const {
    static_assert(std::is_trivially_copyable_v<T>, "Type must be trivially copyable");
    static_assert(std::is_standard_layout_v<T>, "Type must be standard layout");

    if (!m_mapped) {
        throw std::runtime_error("Buffer::ReadAt : Buffer not mapped");
    }

    if (sizeof(T) != m_instanceSize) {
        throw std::runtime_error("Buffer::ReadAt : Size of type does not match instance size");
    }

    return *reinterpret_cast<T *>(static_cast<uint8_t *>(m_mapped) + index * m_alignmentSize);
}

template<typename T>
void Memory::Buffer::WriteAt(const uint32_t index, const T &data) {
    static_assert(std::is_trivially_copyable_v<T>, "Type must be trivially copyable");
    static_assert(std::is_standard_layout_v<T>, "Type must be standard layout");

    if (!m_mapped) {
        throw std::runtime_error("Buffer::WriteAt : Buffer not mapped");
    }

    if (sizeof(T) != m_instanceSize) {
        throw std::runtime_error("Buffer::ReadAt : Size of type does not match instance size");
    }

    *reinterpret_cast<T *>(static_cast<uint8_t *>(m_mapped) + index * m_alignmentSize) = data;
}