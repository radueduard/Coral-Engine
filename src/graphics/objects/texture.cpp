//
// Created by radue on 11/12/2024.
//

#include "texture.h"

#include <boost/uuid/random_generator.hpp>

#include "assets/manager.h"
#include "memory/buffer.h"
#include "memory/sampler.h"

namespace mgv {
    Texture::Texture(const Builder &builder) : m_name(builder.m_name) {
        const auto extent = vk::Extent3D(builder.m_width, builder.m_height, 1);
        const auto mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(builder.m_width, builder.m_height)))) + 1;
        m_image = Memory::Image::Builder()
            .Format(builder.m_format)
            .Extent(extent)
            .MipLevels(mipLevels)
            .SampleCount(vk::SampleCountFlagBits::e1)
            .InitialLayout(vk::ImageLayout::eUndefined)
            .UsageFlags(vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled)
            .ViewType(vk::ImageViewType::e2D)
            .Build();

        if (builder.m_data) {
            auto stagingBuffer = Memory::Buffer<glm::u8vec4>(
                builder.m_width * builder.m_height,
                vk::BufferUsageFlagBits::eTransferSrc,
                vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

            stagingBuffer.Map();
            stagingBuffer.Write(reinterpret_cast<const glm::u8vec4*>(builder.m_data));
            stagingBuffer.Flush();
            stagingBuffer.Unmap();

            m_image->TransitionLayout(vk::ImageLayout::eTransferDstOptimal);
            m_image->Copy(*stagingBuffer);
        }
        m_image->TransitionLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

        m_descriptorInfo = vk::DescriptorImageInfo()
            .setSampler(Memory::Sampler::Get(vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerAddressMode::eRepeat, vk::SamplerMipmapMode::eLinear))
            .setImageView(m_image->ImageView())
            .setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
    }
}
