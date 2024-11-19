//
// Created by radue on 11/17/2024.
//

#include "sampler.h"

namespace Memory {
    std::vector<std::unique_ptr<Sampler>> Sampler::m_samplers;

    Sampler::Sampler(Core::Device &device, const vk::Filter magFilter, const vk::Filter minFilter, const vk::SamplerAddressMode addressMode, const vk::SamplerMipmapMode mipmapMode)
        : m_device(device), m_magFilter(magFilter), m_minFilter(minFilter), m_addressMode(addressMode), m_mipmapMode(mipmapMode) {
        const auto samplerInfo = vk::SamplerCreateInfo()
                .setMagFilter(m_magFilter)
                .setMinFilter(m_minFilter)
                .setAddressModeU(m_addressMode)
                .setAddressModeV(m_addressMode)
                .setAddressModeW(m_addressMode)
                .setAnisotropyEnable(vk::True)
                .setMaxAnisotropy(16)
                .setBorderColor(vk::BorderColor::eIntOpaqueBlack)
                .setUnnormalizedCoordinates(vk::False)
                .setCompareEnable(vk::False)
                .setCompareOp(vk::CompareOp::eAlways)
                .setMipmapMode(m_mipmapMode)
                .setMipLodBias(0.0f)
                .setMinLod(0.0f)
                .setMaxLod(vk::LodClampNone);

        m_sampler = (*m_device).createSampler(samplerInfo);
    }

    Sampler::~Sampler() {
        (*m_device).destroySampler(m_sampler);
    }

    vk::Sampler Sampler::Get(Core::Device& device, const vk::Filter magFilter, const vk::Filter minFilter,
        const vk::SamplerAddressMode addressMode, const vk::SamplerMipmapMode mipmapMode) {

        for (const auto &sampler : m_samplers) {
            if (sampler->MagFilter() == magFilter && sampler->MinFilter() == minFilter &&
                sampler->AddressMode() == addressMode && sampler->MipmapMode() == mipmapMode) {
                return **sampler;
            }
        }

        auto sampler = std::make_unique<Sampler>(device, magFilter, minFilter, addressMode, mipmapMode);
        m_samplers.push_back(std::move(sampler));
        return **m_samplers.back();
    }

    void Sampler::FreeAllSamplers() {
        m_samplers.clear();
    }
}
