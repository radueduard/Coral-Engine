//
// Created by radue on 11/17/2024.
//

#include "sampler.h"

#include "context.h"
#include "core/device.h"

namespace Coral::Memory {
    Sampler::Sampler(const Builder& builder)
        : m_magFilter(builder.magFilter), m_minFilter(builder.minFilter), m_addressMode(builder.addressMode), m_mipmapMode(builder.mipmapMode) {
        const auto samplerInfo = vk::SamplerCreateInfo()
            .setMagFilter(m_magFilter)
            .setMinFilter(m_minFilter)
            .setAddressModeU(m_addressMode)
            .setAddressModeV(m_addressMode)
            .setAddressModeW(m_addressMode)
            .setAnisotropyEnable(vk::False)
            .setMaxAnisotropy(16)
            .setBorderColor(vk::BorderColor::eIntOpaqueBlack)
            .setUnnormalizedCoordinates(vk::False)
            .setCompareEnable(vk::False)
            .setCompareOp(vk::CompareOp::eAlways)
            .setMipmapMode(m_mipmapMode)
            .setMipLodBias(0.0f)
            .setMinLod(0.0f)
            .setMaxLod(vk::LodClampNone);

        m_handle = Context::Device()->createSampler(samplerInfo);
    }

    Sampler::~Sampler() {
        Context::Device()->destroySampler(m_handle);
    }
}
