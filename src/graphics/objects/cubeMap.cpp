//
// Created by radue on 11/29/2024.
//

#include "cubeMap.h"

#include <stb_image.h>
#include <glm/glm.hpp>

#include "memory/buffer.h"
#include "memory/image.h"
#include "memory/sampler.h"

namespace Coral::Graphics {
    CubeMap::CubeMap(const Builder &builder) {
        int width, height;
        const std::array paths {
            builder.m_positiveX,
            builder.m_negativeX,
            builder.m_positiveY,
            builder.m_negativeY,
            builder.m_positiveZ,
            builder.m_negativeZ
        };

        std::array<stbi_uc*, 6> colors { nullptr };
        for (size_t i = 0; i < 6; i++) {
            colors[i] = stbi_load(paths[i].c_str(), &width, &height, nullptr, STBI_rgb_alpha);
            if (width != height) {
                throw std::runtime_error("Cubemap images must be square");
            }
            if (colors[i] == nullptr) {
                throw std::runtime_error("Failed to load image");
            }
        }

        m_size = static_cast<uint32_t>(width);
        m_image = Memory::Image::Builder()
            .Format(vk::Format::eR8G8B8A8Srgb)
            .Extent({ m_size, m_size, 1 })
            .UsageFlags(vk::ImageUsageFlagBits::eSampled)
    		.UsageFlags(vk::ImageUsageFlagBits::eTransferDst)
            .MipLevels(1)
            .LayersCount(6)
            .SampleCount(vk::SampleCountFlagBits::e1)
            .InitialLayout(vk::ImageLayout::eTransferDstOptimal)
            .Build();

        const auto stagingBuffer = Memory::Buffer::Builder()
    		.InstanceSize(sizeof(Math::Vector4<u8>))
            .InstanceCount(m_size * m_size)
            .UsageFlags(vk::BufferUsageFlagBits::eTransferSrc)
            .MemoryProperty(vk::MemoryPropertyFlagBits::eHostVisible)
            .MemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent)
            .Build();

        for (uint32_t i = 0; i < 6; i++) {
            stagingBuffer->Map<Math::Vector4<u8>>();
            std::span data(reinterpret_cast<Math::Vector4<u8>*>(colors[i]), m_size * m_size);
            stagingBuffer->Write(data);
            stagingBuffer->Unmap();

            m_image->Copy(**stagingBuffer, 0, i);
        }

        m_image->TransitionLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

        m_imageViews.emplace_back(Memory::ImageView::Builder(*m_image)
            .ViewType(vk::ImageViewType::eCube)
            .BaseArrayLayer(0)
            .LayerCount(6)
            .Build());

        const auto samplerCreateInfo = Memory::Sampler::CreateInfo {
            .magFilter = vk::Filter::eLinear,
            .minFilter = vk::Filter::eLinear,
            .addressMode = vk::SamplerAddressMode::eRepeat,
            .mipmapMode = vk::SamplerMipmapMode::eLinear
        };
        m_sampler = std::make_unique<Memory::Sampler>(samplerCreateInfo);
    }

    vk::DescriptorImageInfo CubeMap::DescriptorInfo() const {
        return vk::DescriptorImageInfo()
                .setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
                .setImageView(**m_imageViews[0])
                .setSampler(**m_sampler);
    }
}
