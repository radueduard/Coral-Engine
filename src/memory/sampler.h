//
// Created by radue on 11/17/2024.
//

#pragma once

#include <vulkan/vulkan.hpp>

namespace Core {
    class Device;
}

namespace Memory {

class Sampler {
    public:
        struct CreateInfo {
            vk::Filter magFilter = vk::Filter::eLinear;
            vk::Filter minFilter = vk::Filter::eLinear;
            vk::SamplerAddressMode addressMode = vk::SamplerAddressMode::eRepeat;
            vk::SamplerMipmapMode mipmapMode = vk::SamplerMipmapMode::eLinear;
        };

        Sampler(const Core::Device& device, const CreateInfo& createInfo);
        ~Sampler();

        [[nodiscard]] vk::Filter MagFilter() const { return m_magFilter; }
        [[nodiscard]] vk::Filter MinFilter() const { return m_minFilter; }
        [[nodiscard]] vk::SamplerAddressMode AddressMode() const { return m_addressMode; }
        [[nodiscard]] vk::SamplerMipmapMode MipmapMode() const { return m_mipmapMode; }

        [[nodiscard]] const vk::Sampler& Handle() const { return m_sampler; }

    private:
        const Core::Device& m_device;

        vk::Sampler m_sampler;

        vk::Filter m_magFilter = vk::Filter::eLinear;
        vk::Filter m_minFilter = vk::Filter::eLinear;
        vk::SamplerAddressMode m_addressMode = vk::SamplerAddressMode::eRepeat;
        vk::SamplerMipmapMode m_mipmapMode = vk::SamplerMipmapMode::eLinear;
    };
}
