//
// Created by radue on 10/20/2024.
//

#pragma once

#include <core/device.h>

namespace Coral::Memory::Descriptor {
    class SetLayout final : public EngineWrapper<vk::DescriptorSetLayout> {
    public:
        class Builder {
            friend class SetLayout;
        public:
            Builder &AddBinding(uint32_t binding, vk::DescriptorType type, vk::ShaderStageFlags stageFlags, uint32_t count = 1);

            [[nodiscard]] bool HasBinding(uint32_t binding) const;
            [[nodiscard]] vk::DescriptorSetLayoutBinding &Binding(uint32_t binding);
            [[nodiscard]] std::unique_ptr<SetLayout> Build() const;

        private:
            std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding> m_bindings;
        };

        explicit SetLayout(const Builder &builder);
        ~SetLayout() override;
        SetLayout(const SetLayout &) = delete;
        SetLayout &operator=(const SetLayout &) = delete;

        [[nodiscard]] bool HasBinding(const uint32_t binding) const { return m_bindings.contains(binding); }
        [[nodiscard]] const vk::DescriptorSetLayoutBinding &Binding(const uint32_t binding) const { return m_bindings.at(binding); }
        [[nodiscard]] const std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding> &Bindings() const { return m_bindings; }

    private:
        std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding> m_bindings;
    };
}
