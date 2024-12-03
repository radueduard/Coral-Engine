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

namespace mgv {
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

            auto stagingBuffer = Memory::Buffer<glm::u8vec4>(
                width * height,
                vk::BufferUsageFlagBits::eTransferSrc,
                vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

            stagingBuffer.Map();
            stagingBuffer.Write(reinterpret_cast<const glm::u8vec4*>(image));
            stagingBuffer.Unmap();

            m_image->Copy(*stagingBuffer, 0, i, tid);
            stbi_image_free(image);
        }
    }

    TextureArray::TextureArray(const Builder &builder)
        : m_name(builder.m_name), m_format(builder.m_format), m_width(builder.m_width), m_height(builder.m_height)
    {
        const auto extent = vk::Extent3D(builder.m_width, builder.m_height, 1);
        const auto mipLevels = builder.m_createMipmaps ? static_cast<uint32_t>(std::ceil(std::log2(std::max(builder.m_width, builder.m_height)))) + 1 : 1;
        m_image = Memory::Image::Builder()
            .Format(builder.m_format)
            .Extent(extent)
            .LayersCount(static_cast<uint32_t>(builder.m_images.size() + builder.m_data.size()))
            .MipLevels(mipLevels)
            .SampleCount(vk::SampleCountFlagBits::e1)
            .InitialLayout(vk::ImageLayout::eUndefined)
            .UsageFlags(vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eSampled)
            .ViewType(vk::ImageViewType::e2DArray)
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
            const auto stagingBuffer = std::make_unique<Memory::Buffer<glm::u8vec4>>(
                builder.m_width * builder.m_height,
                vk::BufferUsageFlagBits::eTransferSrc,
                vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

            stagingBuffer->Map();
            stagingBuffer->Write(static_cast<const glm::u8vec4*>(builder.m_data[i]));
            stagingBuffer->Unmap();

            m_image->Copy(**stagingBuffer, 0, i + static_cast<uint32_t>(builder.m_images.size()));
        }

        m_image->GenerateMipmaps();

        m_descriptorInfo = vk::DescriptorImageInfo()
            .setSampler(Memory::Sampler::Get(vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerAddressMode::eRepeat, vk::SamplerMipmapMode::eLinear))
            .setImageView(m_image->ImageView())
            .setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
    }
}
