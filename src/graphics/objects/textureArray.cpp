//
// Created by radue on 11/12/2024.
//

#include "textureArray.h"
#include "memory/buffer.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <glm/glm.hpp>

#include <functional>
#include <thread>

#include "memory/sampler.h"

namespace Coral::Graphics {
    uint32_t TextureArray::Id(const std::string &name) const {
        if (!m_imageIndices.contains(name)) {
            return -1;
        }
        return m_imageIndices.at(name);
    }

    void TextureArray::LoadTexture(const ThreadPayload& threadPayload) {
        auto [tid, numThreads, paths] = threadPayload;
        const auto size = static_cast<uint32_t>(paths.size());
        const uint32_t start = tid * size / numThreads;
        uint32_t end = (tid + 1) * size / numThreads;
        end = std::min(end, size);

        for (uint32_t i = start; i < end; i++) {
            const auto& path = paths[i];
            m_imageIndices[path] = i;

            uint32_t width, height, channels;
            stbi_uc* image = stbi_load(path.c_str(), reinterpret_cast<int*>(&width), reinterpret_cast<int*>(&height), reinterpret_cast<int*>(&channels), STBI_rgb_alpha);

            const auto stagingBuffer = Memory::Buffer::Builder()
        		.InstanceSize(sizeof(Math::Vector4<u8>))
                .InstanceCount(width * height)
                .UsageFlags(vk::BufferUsageFlagBits::eTransferSrc)
                .MemoryProperty(vk::MemoryPropertyFlagBits::eHostVisible)
                .MemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent)
                .Build();

            stagingBuffer->Map<Math::Vector4<u8>>();
            const auto copy = std::span(reinterpret_cast<Math::Vector4<u8>*>(image), width * height);
            stagingBuffer->Write(copy);
            stagingBuffer->Unmap();

            m_image->Copy(**stagingBuffer, 0, i);
            stbi_image_free(image);
        }
    }

    std::unique_ptr<TextureArray> TextureArray::Builder::Build() const {
        return std::make_unique<TextureArray>(*this);
    }

    TextureArray::TextureArray(const Builder &builder)
        : m_name(builder.m_name), m_format(builder.m_format), m_width(builder.m_width), m_height(builder.m_height)
    {
        const auto mipLevels = builder.m_createMipmaps ? static_cast<uint32_t>(std::ceil(std::log2(std::max(builder.m_width, builder.m_height)))) + 1 : 1;
        m_image = Memory::Image::Builder()
            .Format(builder.m_format)
            .Extent({ builder.m_width, builder.m_height, 1u })
            .LayersCount(static_cast<uint32_t>(builder.m_images.size() + builder.m_data.size()))
            .MipLevels(mipLevels)
            .SampleCount(vk::SampleCountFlagBits::e1)
            .InitialLayout(vk::ImageLayout::eUndefined)
            .UsageFlags(vk::ImageUsageFlagBits::eTransferDst)
    		.UsageFlags(vk::ImageUsageFlagBits::eTransferSrc)
    		.UsageFlags(vk::ImageUsageFlagBits::eSampled)
            .Build();
        m_image->TransitionLayout(vk::ImageLayout::eTransferDstOptimal);

        const uint32_t numThreads = std::thread::hardware_concurrency();
        std::vector<std::thread> threads;
        std::vector<ThreadPayload> payloads(numThreads);
        for (uint32_t i = 0; i < numThreads; i++) {
            payloads[i] = ThreadPayload {
                .tid = i,
                .numThreads = numThreads,
                .paths = builder.m_images
            };
            std::function func = [this] (const ThreadPayload &payload) { return LoadTexture(payload); };
            threads.emplace_back(func, payloads[i]);
        }

        for (auto& thread : threads) {
            thread.join();
        }

        for (uint32_t i = 0; i < builder.m_data.size(); i++) {
            const auto stagingBuffer = Memory::Buffer::Builder()
        		.InstanceSize(sizeof(Math::Vector4<u8>))
                .InstanceCount(builder.m_width * builder.m_height)
                .UsageFlags(vk::BufferUsageFlagBits::eTransferSrc)
                .MemoryProperty(vk::MemoryPropertyFlagBits::eHostVisible)
                .MemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent)
                .Build();

            stagingBuffer->Map<Math::Vector4<u8>>();
            const auto copy = std::span(static_cast<Math::Vector4<u8>*>(builder.m_data[i]), builder.m_width * builder.m_height);
            stagingBuffer->Write(copy);
            stagingBuffer->Unmap();

            m_image->Copy(**stagingBuffer, 0, i + static_cast<uint32_t>(builder.m_images.size()));
        }

        m_image->GenerateMipmaps();

        m_imageViews.emplace_back(Memory::ImageView::Builder(*m_image)
            .ViewType(vk::ImageViewType::e2DArray)
            .BaseMipLevel(0)
            .LevelCount(mipLevels)
            .BaseArrayLayer(0)
            .LayerCount(static_cast<uint32_t>(builder.m_images.size() + builder.m_data.size()))
            .Build());

        constexpr auto samplerCreateInfo = Memory::Sampler::CreateInfo {
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
}
