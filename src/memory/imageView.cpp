//
// Created by radue on 2/10/2025.
//

#include "imageView.h"

#include "core/device.h"
#include "memory/image.h"

Memory::ImageView::ImageView(const Builder &builder)
    : m_image(builder.m_image), m_viewType(builder.m_viewType),
      m_baseMipLevel(builder.m_baseMipLevel), m_mipLevelCount(builder.m_levelCount),
      m_baseArrayLayer(builder.m_baseArrayLayer), m_arrayLayerCount(builder.m_layerCount) {
    vk::ImageAspectFlags aspectMask = {};
    if (m_image.UsageFlags() & vk::ImageUsageFlagBits::eDepthStencilAttachment) {
        aspectMask = vk::ImageAspectFlagBits::eDepth;
        if (m_image.Format() == vk::Format::eD32SfloatS8Uint || m_image.Format() == vk::Format::eD24UnormS8Uint) {
            aspectMask |= vk::ImageAspectFlagBits::eStencil;
        }
    } else {
        aspectMask = vk::ImageAspectFlagBits::eColor;
    }

    const auto viewInfo = vk::ImageViewCreateInfo()
        .setImage(*m_image)
        .setViewType(builder.m_viewType)
        .setFormat(m_image.Format())
        .setSubresourceRange(vk::ImageSubresourceRange()
            .setAspectMask(aspectMask)
            .setBaseMipLevel(builder.m_baseMipLevel)
            .setLevelCount(builder.m_levelCount)
            .setBaseArrayLayer(builder.m_baseArrayLayer)
            .setLayerCount(builder.m_layerCount));

    m_handle = Core::GlobalDevice()->createImageView(viewInfo);
}

Memory::ImageView::~ImageView() {
    Core::GlobalDevice()->destroyImageView(m_handle);
}

bool Memory::ImageView::Has(
    const uint32_t baseArrayLayer, const uint32_t arrayLayerCount,
    const uint32_t baseMipLevel, const uint32_t mipLevelCount) const {
    return baseArrayLayer == m_baseArrayLayer && arrayLayerCount == m_arrayLayerCount &&
        baseMipLevel == m_baseMipLevel && mipLevelCount == m_mipLevelCount;
}
