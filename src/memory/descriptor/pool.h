//
// Created by radue on 10/20/2024.
//

#pragma once

#include "setLayout.h"

#include <vector>

#include "context.h"

namespace Coral::Memory::Descriptor {
    class Pool {
    public:
        class Builder {
            friend class Pool;
        public:
            Builder &AddPoolSize(vk::DescriptorType type, uint32_t count);
            Builder &PoolFlags(vk::DescriptorPoolCreateFlags flags);
            Builder &MaxSets(uint32_t count);
            [[nodiscard]] std::unique_ptr<Pool> Build() const;

        private:
            std::vector<vk::DescriptorPoolSize> m_poolSizes = {};
            vk::DescriptorPoolCreateFlags m_flags = {};
            uint32_t m_maxSets = 1000;
        };

        [[nodiscard]] vk::DescriptorPool Handle() const { return m_pool; }

        explicit Pool(const Builder &builder);
        ~Pool();
        Pool(const Pool &) = delete;
        Pool &operator=(const Pool &) = delete;

        [[nodiscard]] vk::DescriptorSet Allocate(const SetLayout &layout) const;
        [[nodiscard]] std::vector<vk::DescriptorSet> Allocate(const std::vector<SetLayout> &layouts) const;

        void Free(const vk::DescriptorSet &descriptorSet) const;
        void Free(const std::vector<vk::DescriptorSet> &descriptorSets) const;

        void Reset() const { Context::Device()->resetDescriptorPool(m_pool); }

    private:
        vk::DescriptorPool m_pool;
        std::vector<vk::DescriptorPoolSize> m_poolSizes;
        vk::DescriptorPoolCreateFlags m_flags;
        uint32_t m_maxSets;
    };
}
