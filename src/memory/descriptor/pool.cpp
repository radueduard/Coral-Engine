//
// Created by radue on 10/20/2024.
//

#include "pool.h"

namespace Memory::Descriptor {
    Pool::Builder & Pool::Builder::AddPoolSize(const vk::DescriptorType type, const uint32_t count) {
        const auto poolSize = vk::DescriptorPoolSize()
            .setType(type)
            .setDescriptorCount(count);
        m_poolSizes.emplace_back(poolSize);
        return *this;
    }

    Pool::Builder & Pool::Builder::PoolFlags(const vk::DescriptorPoolCreateFlags flags) {
        m_flags = flags;
        return *this;
    }

    Pool::Builder & Pool::Builder::MaxSets(const uint32_t count) {
        m_maxSets = count;
        return *this;
    }

    std::unique_ptr<Pool> Pool::Builder::Build() const {
        return std::make_unique<Pool>(*this);
    }

    Pool::Pool(const Builder &builder) :
        m_device(builder.m_device),
        m_poolSizes(builder.m_poolSizes),
        m_flags(builder.m_flags),
        m_maxSets(builder.m_maxSets)
    {
        const auto poolCreateInfo = vk::DescriptorPoolCreateInfo()
            .setPoolSizes(m_poolSizes)
            .setMaxSets(m_maxSets)
            .setFlags(m_flags);

        m_pool = (*m_device).createDescriptorPool(poolCreateInfo);
    }

    Pool::~Pool() {
        (*m_device).destroyDescriptorPool(m_pool);
    }

    vk::DescriptorSet Pool::Allocate(const SetLayout &layout) const {
        const auto layoutHandle = *layout;
        const auto allocateInfo = vk::DescriptorSetAllocateInfo()
            .setDescriptorPool(m_pool)
            .setSetLayouts({layoutHandle});
        return (*m_device).allocateDescriptorSets(allocateInfo).front();
    }

    std::vector<vk::DescriptorSet> Pool::Allocate(const std::vector<SetLayout> &layouts) const {
        auto layoutHandles = std::vector<vk::DescriptorSetLayout>();
        for (const auto &layout: layouts) {
            layoutHandles.emplace_back(*layout);
        }

        const auto allocateInfo = vk::DescriptorSetAllocateInfo()
            .setDescriptorPool(m_pool)
            .setSetLayouts(layoutHandles);

        return (*m_device).allocateDescriptorSets(allocateInfo);
    }

    void Pool::Free(const vk::DescriptorSet &descriptorSet) const {
        (*m_device).freeDescriptorSets(m_pool, descriptorSet);
    }

    void Pool::Free(const std::vector<vk::DescriptorSet> &descriptorSets) const {
        (*m_device).freeDescriptorSets(m_pool, descriptorSets);
    }

}
