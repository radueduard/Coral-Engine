//
// Created by radue on 10/24/2024.
//

#pragma once

#include <optional>
#include <vulkan/vulkan.hpp>

namespace Core {
    class Device;
}

namespace Memory {
    class Image {
    public:
        class Builder {
            friend class Image;
        public:
            Builder() = default;

            Builder& Image(const vk::Image image) {
                m_image = image;
                return *this;
            }

            Builder& Format(const vk::Format format) {
                m_format = format;
                return *this;
            }

            Builder& Extent(const vk::Extent3D extent) {
                m_extent = extent;
                return *this;
            }

            Builder& UsageFlags(const vk::ImageUsageFlags usageFlags) {
                m_usageFlags = usageFlags;
                return *this;
            }

            Builder& SampleCount(const vk::SampleCountFlagBits sampleCount) {
                m_sampleCount = sampleCount;
                return *this;
            }

            Builder& MipLevels(const uint32_t mipLevels) {
                m_mipLevels = mipLevels;
                return *this;
            }

            Builder& LayersCount(const uint32_t layersCount) {
                m_layersCount = layersCount;
                return *this;
            }

            Builder& ViewType(const vk::ImageViewType viewType) {
                m_viewType = viewType;
                return *this;
            }

            Builder& InitialLayout(const vk::ImageLayout layout) {
                m_layout = layout;
                return *this;
            }

            std::unique_ptr<Memory::Image> Build() {
                if (m_extent.width == 0 || m_extent.height == 0 || m_extent.depth == 0) {
                    throw std::runtime_error("Image : Extent must be set");
                }
                return std::make_unique<Memory::Image>(*this);
            }
        private:
            vk::Format m_format = vk::Format::eUndefined;
            vk::Extent3D m_extent = vk::Extent3D();
            vk::ImageUsageFlags m_usageFlags = vk::ImageUsageFlagBits::eSampled;
            vk::SampleCountFlagBits m_sampleCount = vk::SampleCountFlagBits::e1;
            uint32_t m_mipLevels = 1;
            uint32_t m_layersCount = 1;
            vk::ImageViewType m_viewType = vk::ImageViewType::e2D;
            vk::ImageLayout m_layout = vk::ImageLayout::eUndefined;

            std::optional<vk::Image> m_image = std::nullopt;
        };

        Image(const Builder& builder);
        ~Image();

        Image(const Image&) = delete;
        Image& operator=(const Image&) = delete;

        const vk::Image& operator*() const { return m_image; }
        [[nodiscard]] const vk::ImageView& ImageView() const { return m_imageView; }
        [[nodiscard]] const vk::ImageLayout& Layout() const { return m_layout; }
        [[nodiscard]] const vk::Extent3D& Extent() const { return m_extent; }

        void Copy(const vk::Buffer& buffer, uint32_t mipLevel = 0, uint32_t layer = 0, uint32_t thread = 0) const;
        void TransitionLayout(vk::ImageLayout newLayout);
        void GenerateMipmaps();
        void Resize(const vk::Extent3D& extent);

        [[nodiscard]] std::vector<vk::ImageView> IndividualMipLevels() const;

    private:
        vk::Image m_image;
        vk::DeviceMemory m_imageMemory;
        vk::ImageView m_imageView;

        vk::Format m_format;
        vk::Extent3D m_extent;
        vk::ImageLayout m_layout = vk::ImageLayout::eUndefined;
        vk::ImageUsageFlags m_usageFlags;
        vk::SampleCountFlagBits m_sampleCount;
        vk::ImageAspectFlags m_aspectMask;

        uint32_t m_mipLevels;
        uint32_t m_layersCount;
    };

}
