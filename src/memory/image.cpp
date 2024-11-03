//
// Created by radue on 10/24/2024.
//

#include "image.h"

#include <iostream>

namespace Memory {
    Image::Image(const Core::Device &device, const Builder &builder)
        : m_device(device), m_format(builder.m_format), m_extent(builder.m_extent), m_layout(builder.m_layout), m_mipLevels(builder.m_mipLevels), m_layersCount(builder.m_layersCount) {
        if (!builder.m_image.has_value()) {
            const vk::ImageCreateFlags imageCreateFlags = builder.m_layersCount == 6 ? vk::ImageCreateFlagBits::eCubeCompatible : vk::ImageCreateFlags();
            const auto imageCreateInfo = vk::ImageCreateInfo()
                .setImageType(vk::ImageType::e2D)
                .setFormat(builder.m_format)
                .setExtent(builder.m_extent)
                .setMipLevels(builder.m_mipLevels)
                .setArrayLayers(builder.m_layersCount)
                .setSamples(builder.m_sampleCount)
                .setTiling(vk::ImageTiling::eOptimal)
                .setUsage(builder.m_usageFlags)
                .setSharingMode(vk::SharingMode::eExclusive)
                .setInitialLayout(vk::ImageLayout::eUndefined)
                .setFlags(imageCreateFlags);

            m_image = (*m_device).createImage(imageCreateInfo);

            const vk::MemoryRequirements memoryRequirements = (*m_device).getImageMemoryRequirements(m_image);
            const auto memoryTypeIndex = m_device.FindMemoryType(memoryRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);
            if (!memoryTypeIndex.has_value()) {
                std::cerr << "Image : Failed to find suitable memory type" << std::endl;
            }

            const auto allocInfo = vk::MemoryAllocateInfo()
                .setAllocationSize(memoryRequirements.size)
                .setMemoryTypeIndex(memoryTypeIndex.value());

            m_imageMemory = (*m_device).allocateMemory(allocInfo);
            (*m_device).bindImageMemory(m_image, m_imageMemory, 0);
        } else {
            m_image = builder.m_image.value();
        }

        vk::ImageAspectFlags aspectMask;
        if (builder.m_usageFlags & vk::ImageUsageFlagBits::eDepthStencilAttachment) {
            aspectMask = vk::ImageAspectFlagBits::eDepth;
            if (builder.m_format == vk::Format::eD32SfloatS8Uint || builder.m_format == vk::Format::eD24UnormS8Uint) {
                aspectMask |= vk::ImageAspectFlagBits::eStencil;
            }
        } else {
            aspectMask = vk::ImageAspectFlagBits::eColor;
        }

        const auto viewInfo = vk::ImageViewCreateInfo()
            .setImage(m_image)
            .setViewType(builder.m_layersCount == 6 ? vk::ImageViewType::eCube : vk::ImageViewType::e2D)
            .setFormat(builder.m_format)
            .setSubresourceRange(vk::ImageSubresourceRange()
                .setAspectMask(aspectMask)
                .setBaseMipLevel(0)
                .setLevelCount(builder.m_mipLevels)
                .setBaseArrayLayer(0)
                .setLayerCount(builder.m_layersCount));

        m_imageView = (*m_device).createImageView(viewInfo);

        if (m_usageFlags & vk::ImageUsageFlagBits::eSampled) {
            const auto samplerInfo = vk::SamplerCreateInfo()
                .setMagFilter(vk::Filter::eLinear)
                .setMinFilter(vk::Filter::eLinear)
                .setAddressModeU(vk::SamplerAddressMode::eClampToEdge)
                .setAddressModeV(vk::SamplerAddressMode::eClampToEdge)
                .setAddressModeW(vk::SamplerAddressMode::eClampToEdge)
                .setAnisotropyEnable(vk::True)
                .setMaxAnisotropy(16)
                .setBorderColor(vk::BorderColor::eIntOpaqueBlack)
                .setUnnormalizedCoordinates(vk::False)
                .setCompareEnable(vk::False)
                .setCompareOp(vk::CompareOp::eAlways)
                .setMipmapMode(vk::SamplerMipmapMode::eLinear)
                .setMipLodBias(0.0f)
                .setMinLod(0.0f)
                .setMaxLod(static_cast<float>(builder.m_mipLevels));

            m_sampler = (*m_device).createSampler(samplerInfo);
        }
    }

    Image::~Image() {
        if (m_sampler) {
            (*m_device).destroySampler(m_sampler);
        }
        (*m_device).destroyImageView(m_imageView);
        if (m_imageMemory) {
            (*m_device).freeMemory(m_imageMemory);
            (*m_device).destroyImage(m_image);
        }
    }

    void Image::Copy(const vk::Buffer &buffer, const uint32_t layer) const {
        const auto commandBuffer = m_device.BeginSingleTimeCommands(Core::Queue::Type::Transfer);
        const auto region = vk::BufferImageCopy()
            .setBufferOffset(0)
            .setBufferRowLength(0)
            .setBufferImageHeight(0)
            .setImageSubresource(vk::ImageSubresourceLayers()
                .setAspectMask(vk::ImageAspectFlagBits::eColor)
                .setMipLevel(0)
                .setBaseArrayLayer(layer)
                .setLayerCount(1))
            .setImageOffset({ 0, 0, 0 })
            .setImageExtent(m_extent);
        commandBuffer.copyBufferToImage(buffer, m_image, vk::ImageLayout::eTransferDstOptimal, region);
        m_device.EndSingleTimeCommands(commandBuffer, Core::Queue::Type::Transfer);
    }

    void Image::TransitionLayout(const vk::ImageLayout newLayout) {
        const auto commandBuffer = m_device.BeginSingleTimeCommands(Core::Queue::Type::Transfer);

        auto barrier = vk::ImageMemoryBarrier()
            .setOldLayout(m_layout)
            .setNewLayout(newLayout)
            .setSrcQueueFamilyIndex(vk::QueueFamilyIgnored)
            .setDstQueueFamilyIndex(vk::QueueFamilyIgnored)
            .setImage(m_image)
            .setSubresourceRange(vk::ImageSubresourceRange()
                .setAspectMask(vk::ImageAspectFlagBits::eColor)
                .setBaseMipLevel(0)
                .setLevelCount(m_mipLevels)
                .setBaseArrayLayer(0)
                .setLayerCount(m_layersCount));

        if (m_format == vk::Format::eD32SfloatS8Uint || m_format == vk::Format::eD24UnormS8Uint) {
            barrier.subresourceRange.setAspectMask(vk::ImageAspectFlagBits::eDepth);
            if (newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal) {
                barrier.subresourceRange.aspectMask |= vk::ImageAspectFlagBits::eStencil;
            }
        } else {
            barrier.subresourceRange.setAspectMask(vk::ImageAspectFlagBits::eColor);
        }

        vk::PipelineStageFlags sourceStage;
        vk::PipelineStageFlags destinationStage;

        switch (m_layout) {
            case vk::ImageLayout::eUndefined:
                barrier.setSrcAccessMask(vk::AccessFlagBits::eNone);
                sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
                switch (newLayout) {
                    case vk::ImageLayout::eTransferDstOptimal:
                        barrier.setDstAccessMask(vk::AccessFlagBits::eTransferWrite);
                        destinationStage = vk::PipelineStageFlagBits::eTransfer;
                    break;
                    case vk::ImageLayout::eDepthStencilAttachmentOptimal:
                        barrier.setDstAccessMask(vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite);
                        destinationStage = vk::PipelineStageFlagBits::eEarlyFragmentTests;
                    break;
                    case vk::ImageLayout::eShaderReadOnlyOptimal:
                        barrier.setDstAccessMask(vk::AccessFlagBits::eShaderRead);
                        destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
                    break;
                    default:
                        throw std::runtime_error("Unsupported new layout transition");
                }
                break;
            case vk::ImageLayout::eTransferDstOptimal:
                barrier.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite);
                sourceStage = vk::PipelineStageFlagBits::eTransfer;
                switch (newLayout) {
                    case vk::ImageLayout::eShaderReadOnlyOptimal:
                        barrier.setDstAccessMask(vk::AccessFlagBits::eShaderRead);
                        destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
                    break;
                    default:
                        throw std::runtime_error("Unsupported new layout transition");
                }
                break;
            case vk::ImageLayout::eDepthStencilAttachmentOptimal:
                barrier.setSrcAccessMask(vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite);
                sourceStage = vk::PipelineStageFlagBits::eEarlyFragmentTests;
                switch (newLayout) {
                    case vk::ImageLayout::eShaderReadOnlyOptimal:
                        barrier.setDstAccessMask(vk::AccessFlagBits::eShaderRead);
                        destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
                    break;
                    default:
                        throw std::runtime_error("Unsupported new layout transition");
                }
                break;
            case vk::ImageLayout::eShaderReadOnlyOptimal:
                barrier.setSrcAccessMask(vk::AccessFlagBits::eShaderRead);
                sourceStage = vk::PipelineStageFlagBits::eFragmentShader;
                switch (newLayout) {
                    case vk::ImageLayout::eTransferDstOptimal:
                        barrier.setDstAccessMask(vk::AccessFlagBits::eTransferWrite);
                        destinationStage = vk::PipelineStageFlagBits::eTransfer;
                    break;
                    default:
                        throw std::runtime_error("Unsupported new layout transition");
                }
                break;
            default:
                throw std::runtime_error("Unsupported old layout transition");
        }

        commandBuffer.pipelineBarrier(
            sourceStage,
            destinationStage,
            vk::DependencyFlags(),
            nullptr,
            nullptr,
            barrier);

        m_layout = newLayout;
        m_device.EndSingleTimeCommands(commandBuffer, Core::Queue::Type::Transfer);
    }


    void Image::GenerateMipmaps() const {
        if (m_mipLevels == 1) {
            return;
        }

        const auto commandBuffer = m_device.BeginSingleTimeCommands(Core::Queue::Type::Transfer);
        auto barrier = vk::ImageMemoryBarrier()
            .setOldLayout(vk::ImageLayout::eUndefined)
            .setNewLayout(vk::ImageLayout::eTransferDstOptimal)
            .setSrcQueueFamilyIndex(vk::QueueFamilyIgnored)
            .setDstQueueFamilyIndex(vk::QueueFamilyIgnored)
            .setImage(m_image)
            .setSubresourceRange(vk::ImageSubresourceRange()
                .setAspectMask(vk::ImageAspectFlagBits::eColor)
                .setBaseMipLevel(0)
                .setLevelCount(1)
                .setBaseArrayLayer(0)
                .setLayerCount(1));

        int32_t mipWidth = m_extent.width;
        int32_t mipHeight = m_extent.height;

        for (uint32_t i = 1; i < m_mipLevels; i++) {
            barrier.subresourceRange.setBaseMipLevel(i - 1);
            barrier.setOldLayout(vk::ImageLayout::eTransferDstOptimal);
            barrier.setNewLayout(vk::ImageLayout::eTransferSrcOptimal);
            barrier.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite);
            barrier.setDstAccessMask(vk::AccessFlagBits::eTransferRead);

            commandBuffer.pipelineBarrier(
                vk::PipelineStageFlagBits::eTransfer,
                vk::PipelineStageFlagBits::eTransfer,
                vk::DependencyFlags(),
                nullptr,
                nullptr,
                barrier);

            const auto imageBlit = vk::ImageBlit()
                .setSrcSubresource(vk::ImageSubresourceLayers()
                    .setAspectMask(vk::ImageAspectFlagBits::eColor)
                    .setMipLevel(i - 1)
                    .setBaseArrayLayer(0)
                    .setLayerCount(1))
                .setSrcOffsets({
                    vk::Offset3D(0, 0, 0),
                    vk::Offset3D(mipWidth, mipHeight, 1)
                })
                .setDstSubresource(vk::ImageSubresourceLayers()
                    .setAspectMask(vk::ImageAspectFlagBits::eColor)
                    .setMipLevel(i)
                    .setBaseArrayLayer(0)
                    .setLayerCount(1))
                .setDstOffsets({
                    vk::Offset3D(0, 0, 0),
                    vk::Offset3D(
                        std::max(1, mipWidth / 2),
                        std::max(1, mipHeight / 2),
                        1)
                });

            commandBuffer.blitImage(
                m_image,
                vk::ImageLayout::eTransferSrcOptimal,
                m_image,
                vk::ImageLayout::eTransferDstOptimal,
                imageBlit,
                vk::Filter::eLinear);

            barrier.setOldLayout(vk::ImageLayout::eTransferSrcOptimal);
            barrier.setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
            barrier.setSrcAccessMask(vk::AccessFlagBits::eTransferRead);
            barrier.setDstAccessMask(vk::AccessFlagBits::eShaderRead);

            commandBuffer.pipelineBarrier(
                vk::PipelineStageFlagBits::eTransfer,
                vk::PipelineStageFlagBits::eFragmentShader,
                vk::DependencyFlags(),
                nullptr,
                nullptr,
                barrier);

            if (mipWidth > 1) {
                mipWidth /= 2;
            }
            if (mipHeight > 1) {
                mipHeight /= 2;
            }

            m_device.EndSingleTimeCommands(commandBuffer, Core::Queue::Type::Transfer);
        }
    }
}