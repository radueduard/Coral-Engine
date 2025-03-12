//
// Created by radue on 11/12/2024.
//

#include "texture.h"

#include <stb_image.h>
#include <glm/glm.hpp>

#include "imgui_impl_vulkan.h"
#include "memory/buffer.h"
#include "memory/image.h"
#include "memory/sampler.h"

namespace mgv {
    Texture::Texture(const Builder &builder)
        : m_name(builder.m_name) {
        const auto extent = vk::Extent3D(builder.m_width, builder.m_height, 1);
        const auto mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(builder.m_width, builder.m_height)))) + 1;
        m_image = Memory::Image::Builder()
            .Format(builder.m_format)
            .Extent(extent)
            .MipLevels(mipLevels)
            .SampleCount(vk::SampleCountFlagBits::e1)
            .InitialLayout(vk::ImageLayout::eUndefined)
            .UsageFlags(vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled)
            .Build();

        if (builder.m_data) {
            const auto stagingBuffer = Memory::Buffer<glm::u8vec4>::Builder()
                .InstanceCount(builder.m_width * builder.m_height)
                .UsageFlags(vk::BufferUsageFlagBits::eTransferSrc)
                .MemoryProperty(vk::MemoryPropertyFlagBits::eHostVisible)
                .MemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent)
                .Build();

            stagingBuffer->Map();
            const auto copy = std::span(reinterpret_cast<glm::u8vec4*>(builder.m_data), builder.m_width * builder.m_height);
            stagingBuffer->Write(copy);
            stagingBuffer->Flush();
            stagingBuffer->Unmap();

            m_image->TransitionLayout(vk::ImageLayout::eTransferDstOptimal);
            m_image->Copy(**stagingBuffer);
        }

        m_image->GenerateMipmaps();
        m_image->TransitionLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

        m_imageViews.emplace_back(Memory::ImageView::Builder(*m_image)
            .ViewType(vk::ImageViewType::e2D)
            .BaseMipLevel(0)
            .LevelCount(mipLevels)
            .Build());

        const auto samplerCreateInfo = Memory::Sampler::CreateInfo {
            .magFilter = vk::Filter::eLinear,
            .minFilter = vk::Filter::eLinear,
            .addressMode = vk::SamplerAddressMode::eRepeat,
            .mipmapMode = vk::SamplerMipmapMode::eLinear,
        };

        m_sampler = std::make_unique<Memory::Sampler>(samplerCreateInfo);

        m_descriptorInfo = vk::DescriptorImageInfo()
            .setSampler(**m_sampler)
            .setImageView(**m_imageViews[0])
            .setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
    }

    std::unique_ptr<Texture> Texture::FromFile(const std::string &path) {
        int width, height, channels;
        stbi_uc* data = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
        if (!data) {
            throw std::runtime_error("Failed to load texture: " + path);
        }

        auto texture = Builder()
            .Name(path)
            .Data(data)
            .Width(width)
            .Height(height)
            .Format(vk::Format::eR8G8B8A8Unorm)
            .Build();

        stbi_image_free(data);
        return texture;
    }
}
