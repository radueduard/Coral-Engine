//
// Created by radue on 10/20/2024.
//

#pragma once

#include "pool.h"
#include "setLayout.h"

namespace Memory::Descriptor {
    class Set {
    public:
        class Builder {
            friend class Set;
        public:
            explicit Builder(const Pool &pool, const SetLayout &layout) : m_pool{pool}, m_layout{layout} {}

            Builder &WriteBuffer(uint32_t binding, const vk::DescriptorBufferInfo& bufferInfo);
            Builder &WriteImage(uint32_t binding, const vk::DescriptorImageInfo& imageInfo);

            [[nodiscard]] std::unique_ptr<Set> Build(const Core::Device& device);

        private:
            const Pool &m_pool;
            const SetLayout &m_layout;
            std::vector<vk::WriteDescriptorSet> m_writes = {};
        };

        [[nodiscard]] vk::DescriptorSet Handle() const { return m_set; }

        explicit Set(const Core::Device& device, Builder &builder);
        ~Set();
        Set(const Set &) = delete;
        Set &operator=(const Set &) = delete;

    private:
        const Core::Device& m_device;
        const Pool &m_pool;
        const SetLayout &m_layout;

        vk::DescriptorSet m_set;
    };
}
