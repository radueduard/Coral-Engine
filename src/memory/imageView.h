//
// Created by radue on 2/10/2025.
//

#pragma once
#include "utils/globalWrapper.h"
#include "vulkan/vulkan.hpp"

namespace Memory {
    class Image;
}

namespace Core {
    class Device;
}

namespace Memory {
    class ImageView final : public EngineWrapper<vk::ImageView> {
    public:
        class Builder {
            friend class ImageView;
        public:
            explicit Builder(const Memory::Image& image) : m_image(image) {}
            ~Builder() = default;

            Builder& ViewType(const vk::ImageViewType viewType) {
                m_viewType = viewType;
                return *this;
            }

            Builder& BaseMipLevel(const uint32_t baseMipLevel) {
                m_baseMipLevel = baseMipLevel;
                return *this;
            }

            Builder& LevelCount(const uint32_t levelCount) {
                m_levelCount = levelCount;
                return *this;
            }

            Builder& BaseArrayLayer(const uint32_t baseArrayLayer) {
                m_baseArrayLayer = baseArrayLayer;
                return *this;
            }

            Builder& LayerCount(const uint32_t layerCount) {
                m_layerCount = layerCount;
                return *this;
            }

            [[nodiscard]] std::unique_ptr<ImageView> Build() const {
                return std::make_unique<ImageView>(*this);
            }

        private:
            const Memory::Image& m_image;
            vk::ImageViewType m_viewType = vk::ImageViewType::e2D;
            uint32_t m_baseMipLevel = 0;
            uint32_t m_levelCount = 1;
            uint32_t m_baseArrayLayer = 0;
            uint32_t m_layerCount = 1;
        };

        explicit ImageView(const Builder& builder);
        ~ImageView() override;

        ImageView(const ImageView&) = delete;
        ImageView& operator=(const ImageView&) = delete;

        [[nodiscard]] const Memory::Image& Image() const { return m_image; }
        [[nodiscard]] const vk::ImageViewType& ViewType() const { return m_viewType; }
        [[nodiscard]] const uint32_t& BaseMipLevel() const { return m_baseMipLevel; }
        [[nodiscard]] const uint32_t& MipLevelCount() const { return m_mipLevelCount; }
        [[nodiscard]] const uint32_t& BaseArrayLayer() const { return m_baseArrayLayer; }
        [[nodiscard]] const uint32_t& ArrayLayerCount() const { return m_arrayLayerCount; }

        bool Has(uint32_t baseArrayLayer, uint32_t arrayLayerCount, uint32_t baseMipLevel, uint32_t mipLevelCount) const;

    private:
        const Memory::Image& m_image;

        vk::ImageViewType m_viewType;
        uint32_t m_baseMipLevel;
        uint32_t m_mipLevelCount;
        uint32_t m_baseArrayLayer;
        uint32_t m_arrayLayerCount;
    };
}
