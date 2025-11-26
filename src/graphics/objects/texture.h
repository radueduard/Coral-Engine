//
// Created by radue on 11/12/2024.
//

#pragma once

#include <memory>
#include <assimp/material.h>
#include <boost/uuid/uuid_generators.hpp>

#include <vulkan/vulkan.hpp>

#include "imgui_impl_vulkan.h"
#include "memory/image.h"
#include "memory/imageView.h"
#include "memory/sampler.h"

namespace Coral::PBR
{
    enum class Usage : uint32_t {
    	None = 0,
        Albedo = 1 << 0,
        Normal = 1 << 1,
        Roughness = 1 << 2,
        Metalic = 1 << 3,
        AmbientOcclusion = 1 << 4,
        Emissive = 1 << 5,
        Height = 1 << 6,
    	MetallicRoughness = Metalic | Roughness,
    };

	inline Usage operator|(Usage lhs, Usage rhs) {
		return static_cast<Usage>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
	}

	inline Usage operator|=(Usage lhs, Usage rhs) {
		return static_cast<Usage>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
	}

	inline Usage operator&(Usage lhs, Usage rhs) {
		return static_cast<Usage>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs));
	}

	inline Usage operator&=(Usage lhs, Usage rhs) {
		return static_cast<Usage>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs));
	}

	inline Usage FromAiTextureType(const aiTextureType type) {
		switch (type) {
			case aiTextureType_NONE: return Usage::None;
			case aiTextureType_BASE_COLOR: return Usage::Albedo;
			case aiTextureType_DIFFUSE: return Usage::Albedo;
			case aiTextureType_NORMALS: return Usage::Normal;
			case aiTextureType_METALNESS: return Usage::Metalic;
			case aiTextureType_DIFFUSE_ROUGHNESS: return Usage::Roughness;
			case aiTextureType_AMBIENT_OCCLUSION: return Usage::AmbientOcclusion;
			case aiTextureType_EMISSIVE: return Usage::Emissive;
			case aiTextureType_HEIGHT: return Usage::Height;
			case aiTextureType_LIGHTMAP: return Usage::AmbientOcclusion;
			default: return Usage::None;
		}
	}
}

namespace Coral::Graphics {
    class Texture {
    public:
        class Builder {
            friend class Texture;
        public:
			explicit Builder(const boost::uuids::uuid uuid = boost::uuids::nil_uuid()) : m_uuid(uuid) {}

            Builder& Name(const std::string& name) {
                m_name = name;
                return *this;
            }

            Builder& Data(u8* data) {
                m_data = data;
                return *this;
            }

        	Builder& Data(const Math::Vector4<u8>* data) {
				m_data = reinterpret_cast<u8*>(const_cast<Math::Vector4<u8>*>(data));
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
                m_usage |= usage;
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
            PBR::Usage m_usage = PBR::Usage::None;
            bool m_createMipmaps = false;
        };

        explicit Texture(const Builder& builder);
        ~Texture() {
	        ImGui_ImplVulkan_RemoveTexture(static_cast<VkDescriptorSet>(m_imId));
        }

        Texture(const Texture&) = delete;
        Texture& operator=(const Texture&) = delete;

        [[nodiscard]] const boost::uuids::uuid& UUID() const { return m_uuid; }
        [[nodiscard]] const std::string& Name() const { return m_name; }
        [[nodiscard]] vk::DescriptorImageInfo DescriptorInfo() const { return m_descriptorInfo; }
        [[nodiscard]] const Math::Vector3<u32>& Extent() const { return m_image->Extent(); }

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
    	[[nodiscard]] ImTextureID ImId() const { return m_imId; }

		[[nodiscard]] std::optional<PBR::Usage> Usage() const { return m_usage; }

		void UpdateDescriptorInfo();
    private:
        boost::uuids::uuid m_uuid;
        std::string m_name;
        vk::DescriptorImageInfo m_descriptorInfo;
        std::optional<PBR::Usage> m_usage = std::nullopt;
    	ImTextureID m_imId = nullptr;

        std::unique_ptr<Memory::Image> m_image;
        std::vector<std::unique_ptr<Memory::ImageView>> m_imageViews;
        std::unique_ptr<Memory::Sampler> m_sampler;
    };
}
