//
// Created by radue on 11/17/2024.
//

#pragma once

#include <vulkan/vulkan.hpp>

#include "utils/globalWrapper.h"

namespace Core {
    class Device;
}

namespace Memory {

class Sampler final : public EngineWrapper<vk::Sampler> {
    public:
        struct CreateInfo {
            vk::Filter magFilter = vk::Filter::eLinear;
            vk::Filter minFilter = vk::Filter::eLinear;
            vk::SamplerAddressMode addressMode = vk::SamplerAddressMode::eRepeat;
            vk::SamplerMipmapMode mipmapMode = vk::SamplerMipmapMode::eLinear;
        };

        explicit Sampler(const CreateInfo& createInfo);
        ~Sampler() override;

        [[nodiscard]] vk::Filter MagFilter() const { return m_magFilter; }
        [[nodiscard]] vk::Filter MinFilter() const { return m_minFilter; }
        [[nodiscard]] vk::SamplerAddressMode AddressMode() const { return m_addressMode; }
        [[nodiscard]] vk::SamplerMipmapMode MipmapMode() const { return m_mipmapMode; }

    private:
        vk::Filter m_magFilter = vk::Filter::eLinear;
        vk::Filter m_minFilter = vk::Filter::eLinear;
        vk::SamplerAddressMode m_addressMode = vk::SamplerAddressMode::eRepeat;
        vk::SamplerMipmapMode m_mipmapMode = vk::SamplerMipmapMode::eLinear;
    };
}
