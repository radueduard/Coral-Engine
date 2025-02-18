//
// Created by radue on 10/24/2024.
//

#include "image.h"

#include <iostream>

#include "core/device.h"

namespace Memory {
    Image::Image(const Core::Device& device, const Builder &builder)
        : m_device(device), m_format(builder.m_format), m_extent(builder.m_extent),
            m_usageFlags(builder.m_usageFlags), m_sampleCount(builder.m_sampleCount),
            m_mipLevels(builder.m_mipLevels), m_layerCount(builder.m_layersCount) {
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

            m_image = m_device.Handle().createImage(imageCreateInfo);

            const vk::MemoryRequirements memoryRequirements = m_device.Handle().getImageMemoryRequirements(m_image);
            const auto memoryTypeIndex = device.FindMemoryType(memoryRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);
            if (!memoryTypeIndex.has_value()) {
                std::cerr << "Image : Failed to find suitable memory type" << std::endl;
            }

            const auto allocInfo = vk::MemoryAllocateInfo()
                .setAllocationSize(memoryRequirements.size)
                .setMemoryTypeIndex(memoryTypeIndex.value());

            m_imageMemory = m_device.Handle().allocateMemory(allocInfo);
            m_device.Handle().bindImageMemory(m_image, m_imageMemory, 0);
        } else {
            m_image = builder.m_image.value();
        }

        if (builder.m_layout != vk::ImageLayout::eUndefined && !builder.m_image.has_value()) {
            TransitionLayout(builder.m_layout);
        }
    }

    Image::~Image() {
        if (m_imageMemory) {
            m_device.Handle().freeMemory(m_imageMemory);
            m_device.Handle().destroyImage(m_image);
        }
    }

    void Image::Copy(const vk::Buffer &buffer, const uint32_t mipLevel, const uint32_t layer, const uint32_t thread) const {
        if (!(m_usageFlags & vk::ImageUsageFlagBits::eTransferDst)) {
            throw std::runtime_error("Image : Image must have transfer destination usage flag");
        }
        if (m_mipLevels <= mipLevel) {
            throw std::runtime_error("Image : Mip level out of range");
        }
        if (m_layerCount <= layer) {
            throw std::runtime_error("Image : Layer out of range");
        }

        m_device.RunSingleTimeCommand([this, buffer, layer, mipLevel] (const vk::CommandBuffer commandBuffer) {
            vk::Extent3D extent = m_extent;
            for (uint32_t i = 0; i < mipLevel; i++) {
                extent.width = std::max<uint32_t>(1u, extent.width / 2);
                extent.height = std::max<uint32_t>(1u, extent.height / 2);
                extent.depth = std::max<uint32_t>(1u, extent.depth / 2);
            }

            const auto region = vk::BufferImageCopy()
                .setBufferOffset(0)
                .setBufferRowLength(0)
                .setBufferImageHeight(0)
                .setImageSubresource(vk::ImageSubresourceLayers()
                    .setAspectMask(vk::ImageAspectFlagBits::eColor)
                    .setMipLevel(mipLevel)
                    .setBaseArrayLayer(layer)
                    .setLayerCount(1))
                .setImageOffset({ 0, 0, 0 })
                .setImageExtent(extent);
            commandBuffer.copyBufferToImage(buffer, m_image, vk::ImageLayout::eTransferDstOptimal, region);
        }, vk::QueueFlagBits::eTransfer, nullptr, thread);
    }

    void Image::TransitionLayout(const vk::ImageLayout newLayout) {
        if (m_layout == newLayout) {
            return;
        }

        m_device.RunSingleTimeCommand([this, newLayout] (const vk::CommandBuffer &commandBuffer) {
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
                    .setLayerCount(m_layerCount));

            if (m_format == vk::Format::eD32SfloatS8Uint || m_format == vk::Format::eD24UnormS8Uint || m_format == vk::Format::eD32Sfloat) {
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
                        case vk::ImageLayout::eColorAttachmentOptimal:
                            barrier.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite);
                            destinationStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
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
                        case vk::ImageLayout::eGeneral:
                            barrier.setDstAccessMask(vk::AccessFlagBits::eMemoryRead | vk::AccessFlagBits::eMemoryWrite);
                            destinationStage = vk::PipelineStageFlagBits::eAllCommands;
                        break;
                        case vk::ImageLayout::eDepthReadOnlyOptimal:
                            barrier.setDstAccessMask(vk::AccessFlagBits::eDepthStencilAttachmentRead);
                            destinationStage = vk::PipelineStageFlagBits::eEarlyFragmentTests;
                        break;
                        case vk::ImageLayout::ePresentSrcKHR:
                            barrier.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead);
                            destinationStage = vk::PipelineStageFlagBits::eTopOfPipe;
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
                        case vk::ImageLayout::eUndefined:
                            barrier.setDstAccessMask(vk::AccessFlagBits::eNone);
                            destinationStage = vk::PipelineStageFlagBits::eTopOfPipe;
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
        }, vk::QueueFlagBits::eGraphics);
    }

    void Image::Barrier(const vk::CommandBuffer &commandBuffer,
        const vk::AccessFlags srcAccessMask, const vk::AccessFlags dstAccessMask,
        const vk::PipelineStageFlags srcStage, const vk::PipelineStageFlags dstStage) const {

        vk::ImageAspectFlags aspectMask = {};
        if (m_usageFlags & vk::ImageUsageFlagBits::eDepthStencilAttachment) {
            aspectMask = vk::ImageAspectFlagBits::eDepth;
            if (m_format == vk::Format::eD32SfloatS8Uint || m_format == vk::Format::eD24UnormS8Uint) {
                aspectMask |= vk::ImageAspectFlagBits::eStencil;
            }
        } else {
            aspectMask = vk::ImageAspectFlagBits::eColor;
        }

        const auto imageBarrier = vk::ImageMemoryBarrier()
            .setOldLayout(m_layout)
            .setNewLayout(m_layout)
            .setImage(m_image)
            .setSubresourceRange(vk::ImageSubresourceRange()
                .setAspectMask(aspectMask)
                .setBaseMipLevel(0)
                .setLevelCount(m_mipLevels)
                .setBaseArrayLayer(0)
                .setLayerCount(m_layerCount))
            .setSrcAccessMask(srcAccessMask)
            .setDstAccessMask(dstAccessMask);

        commandBuffer.pipelineBarrier(
            srcStage,
            dstStage,
            vk::DependencyFlags(),
            nullptr,
            nullptr,
            imageBarrier);
    }

    void Image::Resize(const vk::Extent3D &extent) {
        if (m_extent == extent || (extent.width == 0 || extent.height == 0 || extent.depth == 0))
            return;

        m_device.Handle().freeMemory(m_imageMemory);
        m_device.Handle().destroyImage(m_image);


        m_extent = extent;
        const auto imageCreateInfo = vk::ImageCreateInfo()
            .setImageType(vk::ImageType::e2D)
            .setFormat(m_format)
            .setExtent(m_extent)
            .setMipLevels(m_mipLevels)
            .setArrayLayers(m_layerCount)
            .setSamples(m_sampleCount)
            .setTiling(vk::ImageTiling::eOptimal)
            .setUsage(m_usageFlags)
            .setSharingMode(vk::SharingMode::eExclusive)
            .setInitialLayout(vk::ImageLayout::eUndefined)
            .setFlags(m_layerCount == 6 ? vk::ImageCreateFlagBits::eCubeCompatible : vk::ImageCreateFlags());
        m_image = m_device.Handle().createImage(imageCreateInfo);

        const vk::MemoryRequirements memoryRequirements = m_device.Handle().getImageMemoryRequirements(m_image);
        const auto memoryTypeIndex = m_device.FindMemoryType(memoryRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);
        if (!memoryTypeIndex.has_value()) {
            std::cerr << "Image : Failed to find suitable memory type" << std::endl;
        }

        m_imageMemory = m_device.Handle().allocateMemory(vk::MemoryAllocateInfo()
            .setAllocationSize(memoryRequirements.size)
            .setMemoryTypeIndex(memoryTypeIndex.value()));
        m_device.Handle().bindImageMemory(m_image, m_imageMemory, 0);

        if (m_layout != vk::ImageLayout::eUndefined) {
            const auto layout = m_layout;
            m_layout = vk::ImageLayout::eUndefined;
            TransitionLayout(layout);
        }
    }

    // std::vector<vk::ImageView> Image::IndividualMipLevels() const {
    //     std::vector<vk::ImageView> imageViews;
    //     for (uint32_t i = 0; i < m_mipLevels; i++) {
    //         const auto viewInfo = vk::ImageViewCreateInfo()
    //             .setImage(m_image)
    //             .setViewType(m_layersCount == 6 ? vk::ImageViewType::eCube : vk::ImageViewType::e2D)
    //             .setFormat(m_format)
    //             .setSubresourceRange(vk::ImageSubresourceRange()
    //                 .setAspectMask(m_aspectMask)
    //                 .setBaseMipLevel(i)
    //                 .setLevelCount(1)
    //                 .setBaseArrayLayer(0)
    //                 .setLayerCount(m_layersCount));
    //         imageViews.push_back(m_device.Handle().createImageView(viewInfo));
    //     }
    //     return imageViews;
    // }


    void Image::GenerateMipmaps() {
        if (m_mipLevels == 1) {
            return;
        }

        m_device.RunSingleTimeCommand([this] (const vk::CommandBuffer &commandBuffer) {
            auto mipWidth = static_cast<int32_t>(m_extent.width);
            auto mipHeight = static_cast<int32_t>(m_extent.height);

            auto barrier = vk::ImageMemoryBarrier();
            for (uint32_t i = 1; i < m_mipLevels; i++) {
                barrier = vk::ImageMemoryBarrier()
                    .setOldLayout(vk::ImageLayout::eTransferDstOptimal)
                    .setNewLayout(vk::ImageLayout::eTransferSrcOptimal)
                    .setSrcQueueFamilyIndex(vk::QueueFamilyIgnored)
                    .setDstQueueFamilyIndex(vk::QueueFamilyIgnored)
                    .setImage(m_image)
                    .setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)
                    .setDstAccessMask(vk::AccessFlagBits::eTransferRead)
                    .setSubresourceRange(vk::ImageSubresourceRange()
                        .setAspectMask(vk::ImageAspectFlagBits::eColor)
                        .setBaseMipLevel(i - 1)
                        .setLevelCount(1)
                        .setBaseArrayLayer(0)
                        .setLayerCount(m_layerCount));

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
                        .setLayerCount(m_layerCount))
                    .setSrcOffsets({
                        vk::Offset3D(0, 0, 0),
                        vk::Offset3D(mipWidth, mipHeight, 1)
                    })
                    .setDstSubresource(vk::ImageSubresourceLayers()
                        .setAspectMask(vk::ImageAspectFlagBits::eColor)
                        .setMipLevel(i)
                        .setBaseArrayLayer(0)
                        .setLayerCount(m_layerCount))
                    .setDstOffsets({
                        vk::Offset3D(0, 0, 0),
                        vk::Offset3D(
                            std::max<uint32_t>(1u, mipWidth / 2),
                            std::max<uint32_t>(1u, mipHeight / 2),
                            1)
                    });

                commandBuffer.blitImage(
                    m_image,
                    vk::ImageLayout::eTransferSrcOptimal,
                    m_image,
                    vk::ImageLayout::eTransferDstOptimal,
                    imageBlit,
                    vk::Filter::eLinear);

                barrier = vk::ImageMemoryBarrier()
                    .setOldLayout(vk::ImageLayout::eTransferSrcOptimal)
                    .setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
                    .setSrcQueueFamilyIndex(vk::QueueFamilyIgnored)
                    .setDstQueueFamilyIndex(vk::QueueFamilyIgnored)
                    .setImage(m_image)
                    .setSrcAccessMask(vk::AccessFlagBits::eTransferRead)
                    .setDstAccessMask(vk::AccessFlagBits::eShaderRead)
                    .setSubresourceRange(vk::ImageSubresourceRange()
                        .setAspectMask(vk::ImageAspectFlagBits::eColor)
                        .setBaseMipLevel(i - 1)
                        .setLevelCount(1)
                        .setBaseArrayLayer(0)
                        .setLayerCount(m_layerCount));

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
            }

            barrier = vk::ImageMemoryBarrier()
                .setOldLayout(vk::ImageLayout::eTransferDstOptimal)
                .setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
                .setSrcQueueFamilyIndex(vk::QueueFamilyIgnored)
                .setDstQueueFamilyIndex(vk::QueueFamilyIgnored)
                .setImage(m_image)
                .setSrcAccessMask(vk::AccessFlagBits::eTransferRead)
                .setDstAccessMask(vk::AccessFlagBits::eShaderRead)
                .setSubresourceRange(vk::ImageSubresourceRange()
                    .setAspectMask(vk::ImageAspectFlagBits::eColor)
                    .setBaseMipLevel(m_mipLevels - 1)
                    .setLevelCount(1)
                    .setBaseArrayLayer(0)
                    .setLayerCount(m_layerCount));

            commandBuffer.pipelineBarrier(
                vk::PipelineStageFlagBits::eTransfer,
                vk::PipelineStageFlagBits::eFragmentShader,
                vk::DependencyFlags(),
                nullptr,
                nullptr,
                barrier);
        }, vk::QueueFlagBits::eGraphics);

        m_layout = vk::ImageLayout::eShaderReadOnlyOptimal;
    }
}
