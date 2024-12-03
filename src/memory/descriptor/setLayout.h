//
// Created by radue on 10/20/2024.
//

#pragma once

#include <core/device.h>

namespace Memory::Descriptor {
    class SetLayout {
    public:
        class Builder {
            friend class SetLayout;
        public:
            Builder &AddBinding(uint32_t binding, vk::DescriptorType type, vk::ShaderStageFlags stageFlags, uint32_t count = 1);
            [[nodiscard]] std::unique_ptr<SetLayout> Build() const;

        private:
            std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding> m_bindings;
        };

        vk::DescriptorSetLayout operator *() const { return m_layout; }

        explicit SetLayout(const Builder &builder);
        ~SetLayout();
        SetLayout(const SetLayout &) = delete;
        SetLayout &operator=(const SetLayout &) = delete;

        [[nodiscard]] bool HasBinding(uint32_t binding) const;
        [[nodiscard]] const vk::DescriptorSetLayoutBinding &Binding(uint32_t binding) const;
        [[nodiscard]] const std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding> &Bindings() const { return m_bindings; }

    private:
        vk::DescriptorSetLayout m_layout;
        std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding> m_bindings;
    };
}
