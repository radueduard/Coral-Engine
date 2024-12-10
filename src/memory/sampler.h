//
// Created by radue on 11/17/2024.
//

#pragma once

#include <vector>
#include <memory>

#include <vulkan/vulkan.hpp>

namespace Memory {

class Sampler {
    public:
        explicit Sampler(vk::Filter magFilter, vk::Filter minFilter, vk::SamplerAddressMode addressMode, vk::SamplerMipmapMode mipmapMode);
        ~Sampler();

        [[nodiscard]] vk::Filter MagFilter() const { return m_magFilter; }
        [[nodiscard]] vk::Filter MinFilter() const { return m_minFilter; }

        [[nodiscard]] vk::SamplerAddressMode AddressMode() const { return m_addressMode; }
        [[nodiscard]] vk::SamplerMipmapMode MipmapMode() const { return m_mipmapMode; }

        [[nodiscard]] const vk::Sampler& operator*() const { return m_sampler; }

        static vk::Sampler Get(vk::Filter magFilter, vk::Filter minFilter, vk::SamplerAddressMode addressMode, vk::SamplerMipmapMode mipmapMode);

        static void FreeAllSamplers();
    private:
        vk::Sampler m_sampler;

        vk::Filter m_magFilter = vk::Filter::eLinear;
        vk::Filter m_minFilter = vk::Filter::eLinear;
        vk::SamplerAddressMode m_addressMode = vk::SamplerAddressMode::eRepeat;
        vk::SamplerMipmapMode m_mipmapMode = vk::SamplerMipmapMode::eLinear;

        static std::vector<std::unique_ptr<Sampler>> m_samplers;
    };
}