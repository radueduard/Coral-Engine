//
// Created by radue on 10/20/2024.
//

#include "setLayout.h"

#include <iostream>
#include <ranges>

namespace Coral::Memory::Descriptor {
    SetLayout::Builder & SetLayout::Builder::AddBinding(
        const uint32_t binding,
        const vk::DescriptorType type,
        const vk::ShaderStageFlags stageFlags,
        const uint32_t count)
    {
        if (m_bindings.contains(binding)) {
            std::cerr << "Binding already exists" << std::endl;
        }

        const auto bindingInfo = vk::DescriptorSetLayoutBinding()
            .setBinding(binding)
            .setDescriptorType(type)
            .setDescriptorCount(count)
            .setStageFlags(stageFlags);
        m_bindings.emplace(binding, bindingInfo);
        return *this;
    }

    bool SetLayout::Builder::HasBinding(const uint32_t binding) const {
        return m_bindings.contains(binding);
    }

    vk::DescriptorSetLayoutBinding& SetLayout::Builder::Binding(const uint32_t binding) {
        return m_bindings.at(binding);
    }

    std::unique_ptr<SetLayout> SetLayout::Builder::Build() const {
        return std::make_unique<SetLayout>(*this);
    }

    SetLayout::SetLayout(const Builder &builder) {
        std::vector<vk::DescriptorSetLayoutBinding> bindings;
        for (const auto &binding: builder.m_bindings | std::views::values) {
            bindings.emplace_back(binding);
        }

        const auto layoutCreateInfo = vk::DescriptorSetLayoutCreateInfo()
            .setBindings(bindings);

        m_handle = Core::GlobalDevice()->createDescriptorSetLayout(layoutCreateInfo);
        m_bindings = builder.m_bindings;
    }

    SetLayout::~SetLayout() {
        Core::GlobalDevice()->destroyDescriptorSetLayout(m_handle);
    }
}
