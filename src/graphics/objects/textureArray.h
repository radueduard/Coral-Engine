//
// Created by radue on 11/12/2024.
//

#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <vulkan/vulkan.hpp>

#include "memory/image.h"
#include "memory/imageView.h"

namespace Coral::Memory {
    class Sampler;
}

struct ThreadPayload {
    uint32_t tid;
    uint32_t numThreads;
    std::vector<std::string> paths;
};

namespace Coral::Graphics {
    class TextureArray {
    public:
        class Builder {
            friend class TextureArray;
        public:
            Builder() = default;

            Builder& Name(const std::string& name) {
                m_name = name;
                return *this;
            }

            Builder& ImageSize(const uint32_t size) {
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

            Builder& AddImagePath(std::string path) {
                m_images.emplace_back(std::move(path));
                return *this;
            }

            Builder& AddImageData(void* data) {
                m_data.emplace_back(data);
                return *this;
            }

            Builder& CreateMipmaps() {
                m_createMipmaps = true;
                return *this;
            }

            [[nodiscard]] std::unique_ptr<TextureArray> Build() const;

        private:
            std::string m_name;
            vk::Format m_format = vk::Format::eR8G8B8A8Srgb;
            uint32_t m_width = 1;
            uint32_t m_height = 1;
            bool m_createMipmaps = false;
            std::vector<std::string> m_images;
            std::vector<void*> m_data;
        };

        explicit TextureArray(const Builder& builder);

        [[nodiscard]] const vk::DescriptorImageInfo& DescriptorInfo() const { return m_descriptorInfo; }
        [[nodiscard]] uint32_t Id(const std::string& name) const;

    private:
        void LoadTexture(const ThreadPayload& threadPayload);

        std::string m_name;
        vk::Format m_format;
        uint32_t m_width;
        uint32_t m_height;
        vk::DescriptorImageInfo m_descriptorInfo;

        std::unique_ptr<Memory::Image> m_image;
        std::vector<std::unique_ptr<Memory::ImageView>> m_imageViews;
        std::unique_ptr<Memory::Sampler> m_sampler;

        std::unordered_map<std::string, uint32_t> m_imageIndices;
    };

}
