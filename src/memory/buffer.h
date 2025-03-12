//
// Created by radue on 10/19/2024.
//

#pragma once

#include <iostream>
#include <vulkan/vulkan.hpp>

#include "core/device.h"

static vk::DeviceSize GetAlignment(const vk::DeviceSize size, const vk::DeviceSize alignment) {
    if (alignment > 0) {
        return (size + alignment - 1) & ~(alignment - 1);
    }
    return size;
}


namespace Memory {
    template <typename T, typename = std::enable_if_t<std::is_trivially_copyable_v<T> && std::is_standard_layout_v<T>>>
    class Buffer final : public EngineWrapper<vk::Buffer> {
    public:
        class Builder {
            friend class Buffer;
        public:
            Builder() = default;
            ~Builder() = default;

            Builder& InstanceCount(const uint32_t instanceCount) {
                m_instanceCount = instanceCount;
                return *this;
            }

            Builder& UsageFlags(const vk::BufferUsageFlags usageFlags) {
                m_usageFlags |= usageFlags;
                return *this;
            }

            Builder& MemoryProperty(const vk::MemoryPropertyFlags memoryPropertyFlags) {
                m_memoryPropertyFlags |= memoryPropertyFlags;
                return *this;
            }

            Builder& DeviceAlignment(const vk::DeviceSize deviceAlignment) {
                m_deviceAlignment = deviceAlignment;
                return *this;
            }

            std::unique_ptr<Buffer<T>> Build() {
                return std::make_unique<Buffer<T>>(*this);
            }

        private:
            uint32_t m_instanceCount = 1;
            vk::BufferUsageFlags m_usageFlags = vk::BufferUsageFlags();
            vk::MemoryPropertyFlags m_memoryPropertyFlags = vk::MemoryPropertyFlags();
            vk::DeviceSize m_deviceAlignment = 0;
        };

        explicit Buffer(const Builder& builder)
            : m_instanceCount(builder.m_instanceCount), m_usageFlags(builder.m_usageFlags), m_memoryPropertyFlags(builder.m_memoryPropertyFlags) {

            m_alignmentSize = GetAlignment(sizeof(T), builder.m_deviceAlignment);
            const auto bufferSize = m_alignmentSize * m_instanceCount;

            const auto bufferInfo = vk::BufferCreateInfo()
                .setSize(bufferSize)
                .setUsage(m_usageFlags)
                .setSharingMode(vk::SharingMode::eExclusive);

            m_handle = Core::GlobalDevice()->createBuffer(bufferInfo);

            const auto memRequirements = Core::GlobalDevice()->getBufferMemoryRequirements(m_handle);
            const auto memoryTypeIndex = Core::GlobalDevice().FindMemoryType(memRequirements.memoryTypeBits, m_memoryPropertyFlags);

            if (!memoryTypeIndex.has_value()) {
                std::cerr << "Buffer : Failed to find suitable memory type" << std::endl;
            }

            const auto allocInfo = vk::MemoryAllocateInfo()
                .setAllocationSize(memRequirements.size)
                .setMemoryTypeIndex(memoryTypeIndex.value());

            m_memory = Core::GlobalDevice()->allocateMemory(allocInfo);
            Core::GlobalDevice()->bindBufferMemory(m_handle, m_memory, 0);
        }
        ~Buffer() override {
            Unmap();
            Core::GlobalDevice()->destroyBuffer(m_handle);
            Core::GlobalDevice()->freeMemory(m_memory);
        }

        Buffer(const Buffer &) = delete;
        Buffer &operator=(const Buffer &) = delete;

        std::span<T> Map(vk::DeviceSize instanceCount = vk::WholeSize, const vk::DeviceSize offset = 0) {
            if (!(m_handle && m_memory)) {
                throw std::runtime_error("Buffer::Map : Buffer or memory not ready");
            }

            if (!(m_memoryPropertyFlags & vk::MemoryPropertyFlagBits::eHostVisible)) {
                throw std::runtime_error("Buffer::Map : Memory not host visible");
            }

            if (instanceCount != vk::WholeSize) {
                instanceCount *= m_alignmentSize;
            }

            m_mapped = static_cast<T*>(Core::GlobalDevice()->mapMemory(
                m_memory,
                offset * m_alignmentSize,
                instanceCount,
                vk::MemoryMapFlags()));

            m_mappedRange = vk::MappedMemoryRange()
                .setMemory(m_memory)
                .setSize(instanceCount)
                .setOffset(offset * m_alignmentSize);

            return std::span<T>(static_cast<T *>(m_mapped), instanceCount);
        }

        void Unmap() {
            if (m_mapped) {
                Core::GlobalDevice()->unmapMemory(m_memory);
                m_mapped = nullptr;
            }
        }

        std::span<T> Read(vk::DeviceSize instanceCount = vk::WholeSize, vk::DeviceSize offset = 0) {
            if (!m_mapped) {
                throw std::runtime_error("Buffer::Read : Buffer not mapped");
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

        void Write(const std::span<T>& data, vk::DeviceSize offset = 0) {
            if (!m_mapped) {
                throw std::runtime_error("Buffer::Write : Buffer not mapped");
            }

            if (data.size() + offset > m_instanceCount) {
                throw std::runtime_error("Buffer::Write : Data size exceeds buffer size");
            }

            T* mapped = m_mapped + offset;
            std::copy(data.begin(), data.end(), mapped);
        }

        void Flush(const vk::DeviceSize instanceCount = vk::WholeSize, const vk::DeviceSize offset = 0) const {
            if (!m_mapped) {
                std::cerr << "Buffer::Flush : Buffer not mapped" << std::endl;
                return;
            }

            if (instanceCount == vk::WholeSize) {
                Core::GlobalDevice()->flushMappedMemoryRanges(m_mappedRange);
                return;
            }

            const auto range = vk::MappedMemoryRange()
                .setMemory(m_memory)
                .setSize(instanceCount * m_alignmentSize)
                .setOffset(offset * m_alignmentSize);
            Core::GlobalDevice()->flushMappedMemoryRanges(range);
        }

        [[nodiscard]] vk::DescriptorBufferInfo DescriptorInfo(vk::DeviceSize instanceCount = vk::WholeSize, vk::DeviceSize offset = 0) const {
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

        void Invalidate(const vk::DeviceSize instanceCount = vk::WholeSize, const vk::DeviceSize offset = 0) const {
            if (!m_mapped) {
                std::cerr << "Buffer::Invalidate : Buffer not mapped" << std::endl;
                return;
            }

            if (instanceCount == vk::WholeSize) {
                Core::GlobalDevice()->invalidateMappedMemoryRanges(m_mappedRange);
                return;
            }
            const auto range = vk::MappedMemoryRange()
                .setMemory(m_memory)
                .setSize(instanceCount * m_alignmentSize)
                .setOffset(offset * m_alignmentSize);
            Core::GlobalDevice()->invalidateMappedMemoryRanges(range);
        }

        [[nodiscard]] T ReadAt(const uint32_t index) const {
            static_assert(std::is_trivially_copyable_v<T>, "Type must be trivially copyable");
            static_assert(std::is_standard_layout_v<T>, "Type must be standard layout");

            if (!m_mapped) {
                throw std::runtime_error("Buffer::ReadAt : Buffer not mapped");
            }

            return *reinterpret_cast<T *>(static_cast<uint8_t *>(m_mapped) + index * m_alignmentSize);
        }

        void WriteAt(const uint32_t index, const T& data) {
            static_assert(std::is_trivially_copyable_v<T>, "Type must be trivially copyable");
            static_assert(std::is_standard_layout_v<T>, "Type must be standard layout");

            if (!m_mapped) {
                throw std::runtime_error("Buffer::WriteAt : Buffer not mapped");
            }

            *reinterpret_cast<T *>(static_cast<uint8_t *>(m_mapped) + index * m_alignmentSize) = data;
        }

        void FlushAt(const uint32_t index) const {
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

        [[nodiscard]] vk::DescriptorBufferInfo DescriptorInfoAt(const uint32_t index) const {
            if (index >= m_instanceCount) {
                throw std::runtime_error("Buffer::DescriptorInfoAt : Index out of bounds");
            }
            return DescriptorInfo(1, index);
        }

        void InvalidateAt(const uint32_t index) const {
            if (index >= m_instanceCount) {
                std::cerr << "Buffer::InvalidateAt : Index out of bounds" << std::endl;
                return;
            }
            Invalidate(1, index);
        }

        template <typename U, typename = std::enable_if_t<std::is_trivially_copyable_v<U> && std::is_standard_layout_v<U>>, typename = std::enable_if_t<sizeof(U) == sizeof(T)>>
        void CopyBuffer(const std::unique_ptr<Buffer<U>> &srcBuffer, const vk::DeviceSize instanceCount = vk::WholeSize, vk::DeviceSize srcOffset = 0, vk::DeviceSize dstOffset = 0) const {
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

            Core::GlobalDevice().RunSingleTimeCommand([this, srcOffset, dstOffset, &srcBuffer, size](const Core::CommandBuffer &commandBuffer) {
                const auto copyRegion = vk::BufferCopy()
                    .setSrcOffset(srcOffset)
                    .setDstOffset(dstOffset)
                    .setSize(size);
                commandBuffer->copyBuffer(**srcBuffer, m_handle, 1, &copyRegion);
            }, vk::QueueFlagBits::eTransfer);
        }

        [[nodiscard]] uint32_t InstanceCount() const { return m_instanceCount; }
        [[nodiscard]] vk::DeviceSize AlignmentSize() const { return m_alignmentSize; }
        [[nodiscard]] vk::DeviceSize Size() const { return m_instanceCount * m_alignmentSize; }
        [[nodiscard]] vk::BufferUsageFlags UsageFlags() const { return m_usageFlags; }
        [[nodiscard]] vk::MemoryPropertyFlags MemoryPropertyFlags() const { return m_memoryPropertyFlags; }


    private:
        vk::DeviceMemory m_memory;

        uint32_t m_instanceCount;
        T* m_mapped = nullptr;
        vk::MappedMemoryRange m_mappedRange;

        vk::DeviceSize m_alignmentSize;
        vk::BufferUsageFlags m_usageFlags;
        vk::MemoryPropertyFlags m_memoryPropertyFlags;
    };
}