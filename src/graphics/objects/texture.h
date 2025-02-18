//
// Created by radue on 11/12/2024.
//

#pragma once

#include <memory>
#include <boost/uuid/uuid_generators.hpp>

#include <vulkan/vulkan.hpp>

#include "memory/image.h"
#include "memory/imageView.h"
#include "memory/sampler.h"

namespace mgv {
    class Texture {
    public:
        class Builder {
            friend class Texture;
        public:
            Builder() = default;

            Builder& Name(const std::string& name) {
                m_name = name;
                return *this;
            }

            Builder& Data(const uint8_t* data) {
                m_data = data;
                return *this;
            }

            Builder& Size(const uint32_t size) {
                m_width = size;
                m_height = size;
                return *this;
            }

            Builder& Width(const uint32_t width) {
                m_width = width;
                return *this;
            }

            Builder& Height(const uint32_t height) {
                m_height = height;
                return *this;
            }

            Builder& Format(const vk::Format format) {
                m_format = format;
                return *this;
            }

            Builder& CreateMipmaps() {
                m_createMipmaps = true;
                return *this;
            }

            [[nodiscard]] std::unique_ptr<Texture> Build(const Core::Device& device) const {
                return std::make_unique<Texture>(device, *this);
            }

        private:
            std::string m_name;
            vk::Format m_format = vk::Format::eR8G8B8A8Srgb;
            const uint8_t* m_data = nullptr;
            uint32_t m_width = 1;
            uint32_t m_height = 1;
            bool m_createMipmaps = false;
        };

        Texture(const Core::Device& device, const Builder& builder);
        ~Texture() = default;

        Texture(const Texture&) = delete;
        Texture& operator=(const Texture&) = delete;

        [[nodiscard]] const std::string& Name() const { return m_name; }
        [[nodiscard]] vk::DescriptorImageInfo DescriptorInfo() const { return m_descriptorInfo; }

        [[nodiscard]] vk::Extent3D Extent() const { return m_image->Extent(); }

        [[nodiscard]] const Memory::Image& Image() const { return *m_image; }
        [[nodiscard]] const Memory::ImageView& ImageView(
            const uint32_t baseArrayLayer = 0, uint32_t arrayLayerCount = std::numeric_limits<uint32_t>::max(),
            const uint32_t baseMipLevel = 0, uint32_t mipLevelCount = std::numeric_limits<uint32_t>::max()) {
            if (arrayLayerCount == std::numeric_limits<uint32_t>::max())
                arrayLayerCount = m_image->LayerCount();
            if (mipLevelCount == std::numeric_limits<uint32_t>::max())
                mipLevelCount = m_image->MipLevels();

            if (baseArrayLayer + arrayLayerCount > m_image->LayerCount())
                throw std::runtime_error("The requested array layer interval was not found!");

            if (baseMipLevel + mipLevelCount > m_image->MipLevels())
                throw std::runtime_error("The requested mipMap interval was not found!");

            for (const auto& imageView : m_imageViews) {
                if (imageView->Has(baseArrayLayer, arrayLayerCount, baseMipLevel, mipLevelCount)) {
                    return *imageView;
                }
            }

            m_imageViews.emplace_back(Memory::ImageView::Builder(*m_image)
                .ViewType(vk::ImageViewType::e2D)
                .BaseMipLevel(baseMipLevel)
                .LevelCount(mipLevelCount)
                .BaseArrayLayer(baseArrayLayer)
                .LayerCount(arrayLayerCount)
                .Build(m_device));

            return *m_imageViews.back();

        }
        [[nodiscard]] const Memory::Sampler& Sampler() const { return *m_sampler; }


        static std::unique_ptr<Texture> FromFile(const Core::Device& device, const std::string& path);
    private:
        const Core::Device& m_device;

        std::string m_name;
        vk::DescriptorImageInfo m_descriptorInfo;

        std::unique_ptr<Memory::Image> m_image;
        std::vector<std::unique_ptr<Memory::ImageView>> m_imageViews;
        std::unique_ptr<Memory::Sampler> m_sampler;
    };
}
