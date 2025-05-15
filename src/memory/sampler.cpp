//
// Created by radue on 11/17/2024.
//

#include "sampler.h"

#include "core/device.h"

namespace Coral::Memory {
    Sampler::Sampler(const CreateInfo& createInfo)
        : m_magFilter(createInfo.magFilter), m_minFilter(createInfo.minFilter), m_addressMode(createInfo.addressMode), m_mipmapMode(createInfo.mipmapMode) {
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

        m_handle = Core::GlobalDevice()->createSampler(samplerInfo);
    }

    Sampler::~Sampler() {
        Core::GlobalDevice()->destroySampler(m_handle);
    }
}
