//
// Created by radue on 10/19/2024.
//

#pragma once

#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <iostream>
#include <vulkan/vulkan.hpp>

#include "context.h"
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
        	Builder();
			~Builder();

        	Builder& InstanceSize(u32 instanceSize);

			Builder& InstanceCount(u32 instanceCount);

			Builder& UsageFlags(vk::BufferUsageFlagBits usageFlag);

			Builder& MemoryProperty(vk::MemoryPropertyFlagBits memoryPropertyFlag);

			Builder& DeviceAlignment(vk::DeviceSize deviceAlignment);

			std::unique_ptr<Buffer> Build();

			String m_name;
        	u32 m_instanceSize = 1;
        	u32 m_instanceCount = 1;
        	UnorderedSet<vk::BufferUsageFlagBits> m_usageFlagSet = {};
        	UnorderedSet<vk::MemoryPropertyFlagBits> m_memoryPropertyFlagSet = {};
            vk::DeviceSize m_deviceAlignment = 0;
        };

        explicit Buffer(const Builder& builder);
		~Buffer() override;

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

    		m_mapped = static_cast<T*>(
				Context::Device()->mapMemory(m_memory, offset * m_alignmentSize, instanceCount, vk::MemoryMapFlags()));

    		m_mappedRange =
				vk::MappedMemoryRange().setMemory(m_memory).setSize(instanceCount).setOffset(offset * m_alignmentSize);

    		return std::span<T>(static_cast<T*>(m_mapped), m_instanceCount);
    	}
    	void Unmap() {
    		if (m_mapped) {
    			Context::Device()->unmapMemory(m_memory);
    			m_mapped = nullptr;
    		}
    	}
    	template <typename T>
		std::span<T> Read(vk::DeviceSize instanceCount = vk::WholeSize, vk::DeviceSize offset = 0) const {
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

    		return std::span<T>(static_cast<T*>(m_mapped) + offset, instanceCount);
    	}
    	template <typename T>
		void Write(const std::span<T>& data, u32 offset = 0) {
    		if (!m_mapped) {
    			throw std::runtime_error("Buffer::Write : Buffer not mapped");
    		}

    		if (data.size() + offset > m_instanceCount) {
    			throw std::runtime_error("Buffer::Write : Data size exceeds buffer size");
    		}

    		T* mapped = static_cast<T*>(m_mapped) + offset;
    		std::copy(data.begin(), data.end(), mapped);
    	}

		void Flush(vk::DeviceSize instanceCount = vk::WholeSize, vk::DeviceSize offset = 0) const;

		[[nodiscard]] vk::DescriptorBufferInfo DescriptorInfo(vk::DeviceSize instanceCount = vk::WholeSize, vk::DeviceSize offset = 0) const;

		void Invalidate(vk::DeviceSize instanceCount = vk::WholeSize, vk::DeviceSize offset = 0) const;

    	template <typename T>
		T ReadAt(const u32 index) const {
    		if (!m_mapped) {
    			throw std::runtime_error("Buffer::ReadAt : Buffer not mapped");
    		}

    		return *reinterpret_cast<T*>(static_cast<uint8_t*>(m_mapped) + index * m_alignmentSize);
    	}
    	template <typename T>
		void WriteAt(const uint32_t index, const T& data) {
    		if (!m_mapped) {
    			throw std::runtime_error("Buffer::WriteAt : Buffer not mapped");
    		}

    		*reinterpret_cast<T*>(static_cast<uint8_t*>(m_mapped) + index * m_alignmentSize) = data;
    	}

		void FlushAt(u32 index) const;

		[[nodiscard]] vk::DescriptorBufferInfo DescriptorInfoAt(u32 index) const;

		void InvalidateAt(u32 index) const;

		void CopyBuffer(const std::unique_ptr<Buffer> &srcBuffer, vk::DeviceSize instanceCount = vk::WholeSize, vk::DeviceSize srcOffset = 0, vk::DeviceSize dstOffset = 0) const;

		[[nodiscard]] uint32_t InstanceCount() const;
		[[nodiscard]] vk::DeviceSize AlignmentSize() const;
		[[nodiscard]] vk::DeviceSize Size() const;
		[[nodiscard]] vk::BufferUsageFlags UsageFlags() const;
		[[nodiscard]] vk::MemoryPropertyFlags MemoryPropertyFlags() const;


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