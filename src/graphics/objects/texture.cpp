//
// Created by radue on 11/12/2024.
//

#include "texture.h"

#include "imgui_impl_vulkan.h"
#include "memory/buffer.h"
#include "memory/image.h"
#include "memory/sampler.h"

namespace Coral::Graphics {
    Texture::Texture(const Builder &builder)
        : m_uuid(builder.m_uuid), m_name(builder.m_name), m_usage(builder.m_usage) {
	    const auto extent = vk::Extent3D(builder.m_width, builder.m_height, 1);
    	const auto mipLevels = builder.m_createMipmaps ? static_cast<uint32_t>(std::floor(std::log2(std::max(builder.m_width, builder.m_height)))) + 1 : 1;
    	m_image = Memory::Image::Builder()
			.Format(builder.m_format)
			.Extent({ builder.m_width, builder.m_height, 1 })
			.MipLevels(builder.m_createMipmaps ? mipLevels : 1)
			.SampleCount(vk::SampleCountFlagBits::e1)
			.InitialLayout(vk::ImageLayout::eUndefined)
			.UsageFlags(vk::ImageUsageFlagBits::eTransferSrc)
			.UsageFlags(vk::ImageUsageFlagBits::eTransferDst)
			.UsageFlags(vk::ImageUsageFlagBits::eSampled)
			.Build();

    	if (builder.m_data) {
    		const auto stagingBuffer = Memory::Buffer::Builder()
				.InstanceSize(sizeof(Math::Vector4<u8>))
				.InstanceCount(builder.m_width * builder.m_height)
				.UsageFlags(vk::BufferUsageFlagBits::eTransferSrc)
				.MemoryProperty(vk::MemoryPropertyFlagBits::eHostVisible)
				.MemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent)
				.Build();

    		stagingBuffer->Map<Math::Vector4<u8>>();
    		const auto copy = std::span(reinterpret_cast<Math::Vector4<u8>*>(builder.m_data), builder.m_width * builder.m_height);
    		stagingBuffer->Write(copy);
    		stagingBuffer->Flush();
    		stagingBuffer->Unmap();

    		m_image->TransitionLayout(vk::ImageLayout::eTransferDstOptimal);
    		m_image->Copy(**stagingBuffer);
    	}

    	if (builder.m_createMipmaps)
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

    	m_imId = ImGui_ImplVulkan_AddTexture(
    		**m_sampler,
    		**m_imageViews[0],
    		static_cast<VkImageLayout>(vk::ImageLayout::eShaderReadOnlyOptimal));
    }
}
