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
    Texture::Texture(const Core::Device& device, const Builder &builder)
        : m_device(device), m_name(builder.m_name) {
        const auto extent = vk::Extent3D(builder.m_width, builder.m_height, 1);
        const auto mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(builder.m_width, builder.m_height)))) + 1;
        m_image = Memory::Image::Builder()
            .Format(builder.m_format)
            .Extent(extent)
            .MipLevels(mipLevels)
            .SampleCount(vk::SampleCountFlagBits::e1)
            .InitialLayout(vk::ImageLayout::eUndefined)
            .UsageFlags(vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled)
            .Build(m_device);

        if (builder.m_data) {
            auto stagingBuffer = Memory::Buffer(
                m_device,
                sizeof(glm::u8vec4), builder.m_width * builder.m_height,
                vk::BufferUsageFlagBits::eTransferSrc,
                vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

            stagingBuffer.Map<glm::u8vec4>();
            const auto copy = std::span(reinterpret_cast<const glm::u8vec4*>(builder.m_data), builder.m_width * builder.m_height);
            stagingBuffer.Write(copy);
            stagingBuffer.Flush();
            stagingBuffer.Unmap();

            m_image->TransitionLayout(vk::ImageLayout::eTransferDstOptimal);
            m_image->Copy(*stagingBuffer);
        }

        m_image->GenerateMipmaps();
        // m_image->TransitionLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

        m_imageViews.emplace_back(Memory::ImageView::Builder(*m_image)
            .ViewType(vk::ImageViewType::e2D)
            .BaseMipLevel(0)
            .LevelCount(mipLevels)
            .Build(device));

        const auto samplerCreateInfo = Memory::Sampler::CreateInfo {
            .magFilter = vk::Filter::eLinear,
            .minFilter = vk::Filter::eLinear,
            .addressMode = vk::SamplerAddressMode::eRepeat,
            .mipmapMode = vk::SamplerMipmapMode::eLinear,
        };

        m_sampler = std::make_unique<Memory::Sampler>(m_device, samplerCreateInfo);

        m_descriptorInfo = vk::DescriptorImageInfo()
            .setSampler(m_sampler->Handle())
            .setImageView(m_imageViews[0]->Handle())
            .setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
    }

    std::unique_ptr<Texture> Texture::FromFile(const Core::Device& device, const std::string &path) {
        int width, height, channels;
        const stbi_uc* data = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
        if (!data) {
            throw std::runtime_error("Failed to load texture: " + path);
        }

        auto texture = Builder()
            .Name(path)
            .Data(data)
            .Width(width)
            .Height(height)
            .Format(vk::Format::eR8G8B8A8Srgb)
            .Build(device);

        stbi_image_free(const_cast<stbi_uc*>(data));
        return texture;
    }
}
