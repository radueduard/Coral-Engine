//
// Created by radue on 10/24/2024.
//

#pragma once

#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <optional>
#include <unordered_set>
#include <vulkan/vulkan.hpp>

#include "math/matrix.h"
#include "utils/globalWrapper.h"


namespace Coral::Core {
	class CommandBuffer;
	class Device;
}

namespace Coral::Memory {
	class Buffer;
	class Image final : public EngineWrapper<vk::Image> {
    public:
        struct Builder {
            friend class Image;

            Builder() {
	            m_name = to_string(boost::uuids::random_generator()());
            }

            Builder& Image(const vk::Image image) {
                m_image = image;
                return *this;
            }

            Builder& Format(const vk::Format format) {
                m_format = format;
                return *this;
            }

        	Builder& Type(const vk::ImageType imageType) {
				m_imageType = imageType;
				return *this;
			}

        	Builder& Extent(const u32& extent) {
            	m_extent = { extent, 1u, 1u };
            	return *this;
            }

        	Builder& Extent(const Math::Vector2<u32>& extent) {
            	m_extent = { extent.x, extent.y, 1u };
            	return *this;
            }

            Builder& Extent(const Math::Vector3<u32>& extent) {
	            m_extent = extent;
            	return *this;
            }

            Builder& UsageFlags(const vk::ImageUsageFlagBits usageFlags) {
                m_usageFlagsSet.emplace(usageFlags);
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

            Builder& LayerCount(const uint32_t layersCount) {
                m_layerCount = layersCount;
                return *this;
            }

            Builder& InitialLayout(const vk::ImageLayout layout) {
                m_layout = layout;
                return *this;
            }

            [[nodiscard]] std::unique_ptr<Memory::Image> Build() const {
                if (m_extent.width == 0 || m_extent.height == 0 || m_extent.depth == 0) {
                    throw std::runtime_error("Image : Extent must be set");
                }
                return std::make_unique<Memory::Image>(*this);
            }

        	String m_name;
        	vk::ImageType m_imageType = vk::ImageType::e2D;
            vk::Format m_format = vk::Format::eUndefined;
            Math::Vector3<u32> m_extent = { 1u, 1u, 1u };
        	UnorderedSet<vk::ImageUsageFlagBits> m_usageFlagsSet = {};
            vk::SampleCountFlagBits m_sampleCount = vk::SampleCountFlagBits::e1;
            u32 m_mipLevels = 1;
        	u32 m_maxMips = 1;
            u32 m_layerCount = 1;
            vk::ImageLayout m_layout = vk::ImageLayout::eUndefined;

            std::optional<vk::Image> m_image = std::nullopt;
        };

        explicit Image(const Builder& builder);
        ~Image() override;

        Image(const Image&) = delete;
        Image& operator=(const Image&) = delete;

        [[nodiscard]] const vk::ImageLayout& Layout() const { return m_layout; }
        [[nodiscard]] const Math::Vector3<u32>& Extent() const { return m_extent; }
    	[[nodiscard]] const vk::ImageType& Type() const { return m_imageType; }
        [[nodiscard]] const vk::Format& Format() const { return m_format; }
        [[nodiscard]] const vk::ImageUsageFlags& UsageFlags() const { return m_usageFlags; }
        [[nodiscard]] const vk::SampleCountFlagBits& SampleCount() const { return m_sampleCount; }
        [[nodiscard]] const uint32_t& MipLevels() const { return m_mipLevels; }
        [[nodiscard]] const uint32_t& LayerCount() const { return m_layerCount; }

        void Copy(const Buffer& buffer, uint32_t mipLevel = 0, uint32_t layer = 0) const;

		void TransitionLayout(const Core::CommandBuffer& commandBuffer, vk::ImageLayout newLayout);
		void TransitionLayout(vk::ImageLayout newLayout);

    	void Barrier(
    		const Core::CommandBuffer& commandBuffer,
    		vk::AccessFlags srcAccessMask,
    		vk::AccessFlags dstAccessMask,
    		vk::PipelineStageFlags srcStage,
    		vk::PipelineStageFlags dstStage,
    		vk::ImageLayout newLayout
    	) const;

    	void Clear(const Core::CommandBuffer& commandBuffer, const vk::ClearValue& clearValue) const;
    	void Clear(const vk::ClearValue& clearValue);
        void GenerateMipmaps();
        void Resize(const Math::Vector3<u32>& extent);

    private:
        vk::DeviceMemory m_imageMemory;

    	vk::ImageType m_imageType;
        vk::Format m_format;
        Math::Vector3<u32> m_extent;
        vk::ImageLayout m_layout = vk::ImageLayout::eUndefined;
        vk::ImageUsageFlags m_usageFlags = {};
        vk::SampleCountFlagBits m_sampleCount;

        uint32_t m_mipLevels;
        uint32_t m_layerCount;

    	inline static std::unordered_map<vk::ImageLayout, vk::AccessFlags> layoutAccessMap = {
			{ vk::ImageLayout::eUndefined, vk::AccessFlagBits::eNone },
			{ vk::ImageLayout::eGeneral, vk::AccessFlagBits::eMemoryRead | vk::AccessFlagBits::eMemoryWrite },
			{ vk::ImageLayout::eColorAttachmentOptimal, vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite },
			{ vk::ImageLayout::eDepthStencilAttachmentOptimal, vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite },
			{ vk::ImageLayout::eShaderReadOnlyOptimal, vk::AccessFlagBits::eShaderRead },
			{ vk::ImageLayout::eTransferSrcOptimal, vk::AccessFlagBits::eTransferRead },
			{ vk::ImageLayout::eTransferDstOptimal, vk::AccessFlagBits::eTransferWrite },
			{ vk::ImageLayout::ePresentSrcKHR, vk::AccessFlagBits::eMemoryRead },
		};

    	inline static std::unordered_map<vk::ImageLayout, vk::PipelineStageFlags> layoutPipelineStageMap = {
			{ vk::ImageLayout::eUndefined, vk::PipelineStageFlagBits::eTopOfPipe },
			{ vk::ImageLayout::eGeneral, vk::PipelineStageFlagBits::eAllCommands },
			{ vk::ImageLayout::eColorAttachmentOptimal, vk::PipelineStageFlagBits::eColorAttachmentOutput },
			{ vk::ImageLayout::eDepthStencilAttachmentOptimal, vk::PipelineStageFlagBits::eEarlyFragmentTests },
			{ vk::ImageLayout::eShaderReadOnlyOptimal, vk::PipelineStageFlagBits::eFragmentShader },
			{ vk::ImageLayout::eTransferSrcOptimal, vk::PipelineStageFlagBits::eTransfer },
			{ vk::ImageLayout::eTransferDstOptimal, vk::PipelineStageFlagBits::eTransfer },
			{ vk::ImageLayout::ePresentSrcKHR, vk::PipelineStageFlagBits::eBottomOfPipe },
		};

    	inline static std::unordered_map<vk::ImageUsageFlagBits, vk::ImageAspectFlagBits> usageAspectMap = {
			{ vk::ImageUsageFlagBits::eColorAttachment, vk::ImageAspectFlagBits::eColor },
			{ vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::ImageAspectFlagBits::eDepth },
			{ vk::ImageUsageFlagBits::eSampled, vk::ImageAspectFlagBits::eColor },
			{ vk::ImageUsageFlagBits::eStorage, vk::ImageAspectFlagBits::eColor },
			{ vk::ImageUsageFlagBits::eInputAttachment, vk::ImageAspectFlagBits::eColor },
		};
    };

}
