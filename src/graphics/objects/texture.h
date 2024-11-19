//
// Created by radue on 11/12/2024.
//

#pragma once

#include <memory>
#include <boost/uuid/uuid_generators.hpp>

#include "core/device.h"
#include "memory/image.h"

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

            [[nodiscard]] std::unique_ptr<Texture> Build(Core::Device& device) {
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

        Texture(Core::Device& device, const Builder& builder);

        Texture(const Texture&) = delete;
        Texture& operator=(const Texture&) = delete;

        [[nodiscard]] const std::string& Name() const { return m_name; }
        [[nodiscard]] vk::DescriptorImageInfo DescriptorInfo() const { return m_descriptorInfo; }
    private:
        std::string m_name;
        vk::DescriptorImageInfo m_descriptorInfo;
        std::unique_ptr<Memory::Image> m_image;
    };
}
