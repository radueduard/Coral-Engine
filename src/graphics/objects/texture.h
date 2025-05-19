//
// Created by radue on 11/12/2024.
//

#pragma once

#include <memory>
#include <assimp/material.h>
#include <boost/uuid/uuid_generators.hpp>

#include <vulkan/vulkan.hpp>

#include "memory/image.h"
#include "memory/imageView.h"
#include "memory/sampler.h"

namespace Coral::PBR
{
    class Usage
    {
    public:
        enum class Values {
            Albedo = 1 << 0,
            Normal = 1 << 1,
            Roughness = 1 << 2,
            Metalic = 1 << 3,
            AmbientOcclusion = 1 << 4,
            Emissive = 1 << 5,
            Height = 1 << 6,
        };

        Usage(const Values value) : m_value(value) {}
        Usage(const std::string& value) {
            if (value == "Albedo") m_value = Values::Albedo;
            else if (value == "Normal") m_value = Values::Normal;
            else if (value == "Roughness") m_value = Values::Roughness;
            else if (value == "Metalic") m_value = Values::Metalic;
            else if (value == "AmbientOcclusion") m_value = Values::AmbientOcclusion;
            else if (value == "Emissive") m_value = Values::Emissive;
            else if (value == "Height") m_value = Values::Height;
        }

        Usage(const aiTextureType value) {
            switch (value) {
                case aiTextureType_DIFFUSE:
                case aiTextureType_BASE_COLOR:
                    m_value = Values::Albedo; break;
                case aiTextureType_NORMALS:
                case aiTextureType_NORMAL_CAMERA:
                    m_value = Values::Normal; break;
                case aiTextureType_HEIGHT:
                    m_value = Values::Height; break;
                case aiTextureType_AMBIENT_OCCLUSION:
                case aiTextureType_LIGHTMAP:
                    m_value = Values::AmbientOcclusion; break;
                case aiTextureType_EMISSIVE:
                case aiTextureType_EMISSION_COLOR:
                    m_value = Values::Emissive; break;
                case aiTextureType_METALNESS:
                case aiTextureType_SHININESS:
                    m_value = Values::Metalic; break;
                case aiTextureType_DIFFUSE_ROUGHNESS:
                case aiTextureType_REFLECTION:
                    m_value = Values::Roughness; break;

                default: throw std::runtime_error("Unknown texture type");
            }
        }

        [[nodiscard]] Values Value() const { return m_value; }

        auto operator<=>(const Usage &other) const {
            return m_value <=> other.m_value;
        }

        [[nodiscard]] bool operator==(const Usage &other) const { return m_value == other.m_value; }
        [[nodiscard]] bool operator==(const Values &other) const { return m_value == other; }

        Usage& operator |=(Values other) {
            m_value = static_cast<Values>(static_cast<uint32_t>(m_value) | static_cast<uint32_t>(other));
            return *this;
        }
        Usage& operator |=(const Usage &other) {
            m_value = static_cast<Values>(static_cast<uint32_t>(m_value) | static_cast<uint32_t>(other.m_value));
            return *this;
        }
        Usage& operator &=(Values other)
        {
            m_value = static_cast<Values>(static_cast<uint32_t>(m_value) & static_cast<uint32_t>(other));
            return *this;
        }
        Usage& operator &=(const Usage &other)
        {
            m_value = static_cast<Values>(static_cast<uint32_t>(m_value) & static_cast<uint32_t>(other.m_value));
            return *this;
        }

        uint32_t operator &(const Usage &other) const {
            return static_cast<uint32_t>(m_value) & static_cast<uint32_t>(other.m_value);
        }

        [[nodiscard]] bool Is(Values value) const { return (*this & value) != 0; }

    private:
        Values m_value;
    };
}

namespace Coral::Graphics {
    class Texture {
    public:
        class Builder {
            friend class Texture;
        public:
            Builder(boost::uuids::uuid uuid = boost::uuids::nil_uuid()) : m_uuid(uuid) {}

            Builder& Name(const std::string& name) {
                m_name = name;
                return *this;
            }

            Builder& Data(u8* data) {
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

            Builder& Usage(const PBR::Usage usage) {
                if (m_usage.has_value()) {
                    m_usage.value() |= usage;
                } else {
                    m_usage = usage;
                }
                return *this;
            }

            Builder& CreateMipmaps() {
                m_createMipmaps = true;
                return *this;
            }

            [[nodiscard]] std::unique_ptr<Texture> Build() const {
                return std::make_unique<Texture>(*this);
            }

        private:
            UUID m_uuid;
            String m_name;
            vk::Format m_format = vk::Format::eR8G8B8A8Unorm;
            u8* m_data = nullptr;
            u32 m_width = 1;
            u32 m_height = 1;
            std::optional<PBR::Usage> m_usage = std::nullopt;
            bool m_createMipmaps = false;
        };

        explicit Texture(const Builder& builder);
        ~Texture() = default;

        Texture(const Texture&) = delete;
        Texture& operator=(const Texture&) = delete;

        [[nodiscard]] const boost::uuids::uuid& UUID() const { return m_uuid; }
        [[nodiscard]] const std::string& Name() const { return m_name; }
        [[nodiscard]] vk::DescriptorImageInfo DescriptorInfo() const { return m_descriptorInfo; }
        [[nodiscard]] Math::Vector3<u32> Extent() const { return m_image->Extent(); }

        [[nodiscard]] const Memory::Image& Image() const { return *m_image; }
        [[nodiscard]] const Memory::ImageView& ImageView(
            const uint32_t baseMipLevel = 0, uint32_t mipLevelCount = std::numeric_limits<uint32_t>::max()) {
            if (mipLevelCount == std::numeric_limits<uint32_t>::max())
                mipLevelCount = m_image->MipLevels();

            if (baseMipLevel + mipLevelCount > m_image->MipLevels())
                throw std::runtime_error("The requested mipMap interval was not found!");

            for (const auto& imageView : m_imageViews) {
                if (imageView->Has(0, 1, baseMipLevel, mipLevelCount)) {
                    return *imageView;
                }
            }

            m_imageViews.emplace_back(Memory::ImageView::Builder(*m_image)
                .ViewType(vk::ImageViewType::e2D)
                .BaseMipLevel(baseMipLevel)
                .LevelCount(mipLevelCount)
                .BaseArrayLayer(0)
                .LayerCount(1)
                .Build());

            return *m_imageViews.back();

        }
        [[nodiscard]] const Memory::Sampler& Sampler() const { return *m_sampler; }
    private:
        boost::uuids::uuid m_uuid;
        std::string m_name;
        vk::DescriptorImageInfo m_descriptorInfo;
        std::optional<PBR::Usage> m_usage = std::nullopt;

        std::unique_ptr<Memory::Image> m_image;
        std::vector<std::unique_ptr<Memory::ImageView>> m_imageViews;
        std::unique_ptr<Memory::Sampler> m_sampler;
    };
}

namespace std {
    template<>
    struct hash<Coral::PBR::Usage> {
        size_t operator()(const Coral::PBR::Usage& usage) const noexcept {
            return hash<uint32_t>()(static_cast<uint32_t>(usage.Value()));
        }
    };

    inline string to_string(const Coral::PBR::Usage::Values& usage) {
        switch (usage) {
            case Coral::PBR::Usage::Values::Albedo: return "Albedo";
            case Coral::PBR::Usage::Values::Normal: return "Normal";
            case Coral::PBR::Usage::Values::Roughness: return "Roughness";
            case Coral::PBR::Usage::Values::Metalic: return "Metalic";
            case Coral::PBR::Usage::Values::AmbientOcclusion: return "AmbientOcclusion";
            case Coral::PBR::Usage::Values::Emissive: return "Emissive";
            case Coral::PBR::Usage::Values::Height: return "Height";
            default: throw std::runtime_error("Unknown texture usage");
        }
    }

    inline string to_string(const Coral::PBR::Usage& usage) {
        return to_string(usage.Value());
    }
}
