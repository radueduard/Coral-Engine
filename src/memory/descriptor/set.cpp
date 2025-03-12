//
// Created by radue on 10/20/2024.
//

#include "set.h"

#include <iostream>

namespace Memory::Descriptor {
    Set::Builder & Set::Builder::WriteBuffer(const uint32_t binding, const vk::DescriptorBufferInfo &bufferInfo) {
        if (!m_layout.HasBinding(binding)) {
            std::cerr << "Binding " << binding << " not found in layout" << std::endl;
            return *this;
        }
        auto &bindingInfo = m_layout.Binding(binding);

        const auto write = vk::WriteDescriptorSet()
            .setDstSet(nullptr)
            .setDstBinding(binding)
            .setDstArrayElement(0)
            .setDescriptorType(bindingInfo.descriptorType)
            .setDescriptorCount(1)
            .setBufferInfo(bufferInfo);

        m_writes.emplace_back(write);
        return *this;
    }

    Set::Builder & Set::Builder::WriteImage(const uint32_t binding, const vk::DescriptorImageInfo &imageInfo) {
        if (!m_layout.HasBinding(binding)) {
            std::cerr << "Binding " << binding << " not found in layout" << std::endl;
            return *this;
        }
        auto &bindingInfo = m_layout.Binding(binding);

        const auto write = vk::WriteDescriptorSet()
            .setDstSet(nullptr)
            .setDstBinding(binding)
            .setDstArrayElement(0)
            .setDescriptorType(bindingInfo.descriptorType)
            .setDescriptorCount(1)
            .setImageInfo(imageInfo);

        m_writes.emplace_back(write);
        return *this;
    }

    std::unique_ptr<Set> Set::Builder::Build() {
        return std::make_unique<Set>(*this);
    }


    Set::Set(Builder &builder) : m_pool{builder.m_pool}, m_layout{builder.m_layout} {
        m_set = m_pool.Allocate(m_layout);
        for (auto &write: builder.m_writes) {
            write.setDstSet(m_set);
        }
        Core::GlobalDevice()->updateDescriptorSets(builder.m_writes, {});
    }

    Set::~Set() {
        Core::GlobalDevice()->waitIdle();
        m_pool.Free(m_set);
    }
}
