//
// Created by radue on 10/19/2024.
//

#pragma once

#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <iostream>
#include <vulkan/vulkan.hpp>

#include "core/device.h"

static vk::DeviceSize GetAlignment(const vk::DeviceSize size, const vk::DeviceSize alignment) {
    if (alignment > 0) {
        return (size + alignment - 1) & ~(alignment - 1);
    }
    return size;
}

namespace Coral::Memory {
    class Buffer final : public EngineWrapper<vk::Buffer> {
    public:
        class Builder {
	        friend class Buffer;
        public:
        	Builder() {
        		m_name = to_string(boost::uuids::random_generator()());
        	}
        	~Builder() = default;

        	Builder& InstanceSize(const u32 instanceSize) {
        		m_instanceSize = instanceSize;
        		return *this;
        	}

        	Builder& InstanceCount(const u32 instanceCount) {
        		m_instanceCount = instanceCount;
        		return *this;
        	}

        	Builder& UsageFlags(const vk::BufferUsageFlagBits usageFlag) {
        		m_usageFlagSet.insert(usageFlag);
        		return *this;
        	}

        	Builder& MemoryProperty(const vk::MemoryPropertyFlagBits memoryPropertyFlag) {
        		m_memoryPropertyFlagSet.insert(memoryPropertyFlag);
        		return *this;
        	}

        	Builder& DeviceAlignment(const vk::DeviceSize deviceAlignment) {
        		m_deviceAlignment = deviceAlignment;
        		return *this;
        	}

        	std::unique_ptr<Buffer> Build() {
        		return std::make_unique<Buffer>(*this);
        	}

        	String m_name;
        	u32 m_instanceSize = 1;
        	u32 m_instanceCount = 1;
        	UnorderedSet<vk::BufferUsageFlagBits> m_usageFlagSet = {};
        	UnorderedSet<vk::MemoryPropertyFlagBits> m_memoryPropertyFlagSet = {};
            vk::DeviceSize m_deviceAlignment = 0;
        };

        explicit Buffer(const Builder& builder)
            : m_instanceCount(builder.m_instanceCount) {
        	for (const auto& flag : builder.m_usageFlagSet) {
        		m_usageFlags |= flag;
        	}
        	for (const auto& flag : builder.m_memoryPropertyFlagSet) {
        		m_memoryPropertyFlags |= flag;
        	}

            m_alignmentSize = GetAlignment(builder.m_instanceSize, builder.m_deviceAlignment);
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

    	template <typename T>
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
    		else {
	            instanceCount = m_instanceCount * m_alignmentSize;
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

    	template <typename T>
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

            return std::span<T>(static_cast<T *>(m_mapped) + offset, instanceCount);
        }

    	template<typename T>
        void Write(const std::span<T>& data, u32 offset = 0) {
            if (!m_mapped) {
                throw std::runtime_error("Buffer::Write : Buffer not mapped");
            }

            if (data.size() + offset > m_instanceCount) {
                throw std::runtime_error("Buffer::Write : Data size exceeds buffer size");
            }

            T* mapped = static_cast<T *>(m_mapped) + offset;
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

    	template<typename T>
        [[nodiscard]] T ReadAt(const u32 index) const {
            if (!m_mapped) {
                throw std::runtime_error("Buffer::ReadAt : Buffer not mapped");
            }

            return *reinterpret_cast<T *>(static_cast<uint8_t *>(m_mapped) + index * m_alignmentSize);
        }

    	template <typename T>
        void WriteAt(const uint32_t index, const T& data) {
            if (!m_mapped) {
                throw std::runtime_error("Buffer::WriteAt : Buffer not mapped");
            }

            *reinterpret_cast<T *>(static_cast<uint8_t *>(m_mapped) + index * m_alignmentSize) = data;
        }

        void FlushAt(const u32 index) const {
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

        [[nodiscard]] vk::DescriptorBufferInfo DescriptorInfoAt(const u32 index) const {
            if (index >= m_instanceCount) {
                throw std::runtime_error("Buffer::DescriptorInfoAt : Index out of bounds");
            }
            return DescriptorInfo(1, index);
        }

        void InvalidateAt(const u32 index) const {
            if (index >= m_instanceCount) {
                std::cerr << "Buffer::InvalidateAt : Index out of bounds" << std::endl;
                return;
            }
            Invalidate(1, index);
        }

        void CopyBuffer(const std::unique_ptr<Buffer> &srcBuffer, const vk::DeviceSize instanceCount = vk::WholeSize, vk::DeviceSize srcOffset = 0, vk::DeviceSize dstOffset = 0) const {
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

        u32 m_instanceCount;
        void* m_mapped = nullptr;
        vk::MappedMemoryRange m_mappedRange;

        vk::DeviceSize m_alignmentSize;
        vk::BufferUsageFlags m_usageFlags;
        vk::MemoryPropertyFlags m_memoryPropertyFlags;
    };
}