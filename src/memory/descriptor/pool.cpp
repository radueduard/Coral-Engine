//
// Created by radue on 10/20/2024.
//

#include "pool.h"
#include "context.h"

#include <ranges>

namespace Coral::Memory::Descriptor {
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
        m_poolSizes(builder.m_poolSizes),
        m_flags(builder.m_flags),
        m_maxSets(builder.m_maxSets)
    {
        const auto poolCreateInfo = vk::DescriptorPoolCreateInfo()
            .setPoolSizes(m_poolSizes)
            .setMaxSets(m_maxSets)
            .setFlags(m_flags);

    	for (const auto& size : m_poolSizes) {
			m_allocatedBindingCounts[size.type] = 0;
		}

        m_pool = Context::Device()->createDescriptorPool(poolCreateInfo);
    }

    Pool::~Pool() {
        Context::Device()->destroyDescriptorPool(m_pool);
    }

    vk::DescriptorSet Pool::Allocate(const SetLayout &layout) {
        const auto layoutHandle = *layout;
        const auto allocateInfo = vk::DescriptorSetAllocateInfo()
            .setDescriptorPool(m_pool)
            .setSetLayouts({layoutHandle});

    	for (const auto& binding : layout.Bindings()| std::views::values) {
    		m_allocatedBindingCounts[binding.descriptorType]++;
    	}
		const auto allocatedSets = Context::Device()->allocateDescriptorSets(allocateInfo);
    	if (allocatedSets.empty()) {
    		throw std::runtime_error("Failed to allocate descriptor set!");
    	}

    	m_allocatedSetCount++;

        return allocatedSets[0];
    }

    std::vector<vk::DescriptorSet> Pool::Allocate(const std::vector<SetLayout> &layouts) {
        auto layoutHandles = std::vector<vk::DescriptorSetLayout>();
        for (const auto &layout: layouts) {
            layoutHandles.emplace_back(*layout);
        }

        const auto allocateInfo = vk::DescriptorSetAllocateInfo()
            .setDescriptorPool(m_pool)
            .setSetLayouts(layoutHandles);

    	for (const auto &layout : layouts) {
			for (const auto& binding : layout.Bindings()| std::views::values) {
				m_allocatedBindingCounts[binding.descriptorType]++;
			}
		}
		const auto allocatedSets = Context::Device()->allocateDescriptorSets(allocateInfo);
    	if (allocatedSets.size() != layouts.size()) {
    		throw std::runtime_error("Failed to allocate descriptor sets!");
    	}
    	m_allocatedSetCount += static_cast<u32>(layouts.size());
        return allocatedSets;
    }

    void Pool::Free(const vk::DescriptorSet &descriptorSet) const {
        Context::Device()->freeDescriptorSets(m_pool, descriptorSet);
    }

    void Pool::Free(const std::vector<vk::DescriptorSet> &descriptorSets) const {
        Context::Device()->freeDescriptorSets(m_pool, descriptorSets);
    }
	void Pool::Reset() const { Context::Device()->resetDescriptorPool(m_pool); }

} // namespace Coral::Memory::Descriptor
