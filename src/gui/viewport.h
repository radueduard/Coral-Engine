//
// Created by radue on 3/6/2025.
//

#pragma once
#include <spirv_cross/spirv_common.hpp>

#include "imgui_impl_vulkan.h"
#include "layer.h"
#include "elements/dockable.h"
#include "elements/image.h"
#include "graphics/framebuffer.h"
#include "graphics/renderPass.h"
#include "memory/sampler.h"

namespace GUI {
	class Viewport final : public Layer {
	public:
		explicit Viewport(Graphics::RenderPass& renderPass) : m_renderPass(renderPass) {
			constexpr auto createInfo = Memory::Sampler::CreateInfo {
				.magFilter = vk::Filter::eLinear,
				.minFilter = vk::Filter::eLinear,
				.addressMode = vk::SamplerAddressMode::eClampToEdge,
				.mipmapMode = vk::SamplerMipmapMode::eNearest,
			};

			m_sampler = std::make_unique<Memory::Sampler>(createInfo);
		}

		void OnGUIAttach() override {
			m_guiBuilder["viewport"] = [this] {
				for (const auto texture : m_viewportTextures) {
					ImGui_ImplVulkan_RemoveTexture(texture);
				}
				m_viewportTextures.clear();
				m_viewportTextures.reserve(m_renderPass.ImageCount());
				const uint32_t outputImageIndex = m_renderPass.OutputImageIndex();
				for (uint32_t i = 0; i < m_renderPass.ImageCount(); i++) {
					const auto& framebuffer = m_renderPass.Framebuffer(i);
					m_viewportTextures.emplace_back(ImGui_ImplVulkan_AddTexture(
						**m_sampler,
						*framebuffer.ImageView(outputImageIndex),
						static_cast<VkImageLayout>(vk::ImageLayout::eShaderReadOnlyOptimal)));
				}
				return new Dockable(
					"Main Viewport",
					new Image(m_viewportTextures[0], m_renderPass.Extent(), 0, Image::Cover),
					10.f,
					nullptr);
			};
		}

		void OnGUIDetach() override {
			for (uint32_t i = 0; i < m_renderPass.ImageCount(); i++) {
				ImGui_ImplVulkan_RemoveTexture(m_viewportTextures[i]);
			}
		}

		void OnGUIUpdate() override {
			// const auto imageCount = m_renderPass.ImageCount();
			// const Math::Vector2<uint32_t> extent = GUIObject("viewport")->OuterBounds().max - GUIObject("viewport")->OuterBounds().min;
			// if (m_renderPass.Resize(imageCount, extent)) {
			// 	ResetElement("viewport");
			// }
		}

	private:
		Graphics::RenderPass& m_renderPass;

		std::unique_ptr<Memory::Sampler> m_sampler;
		std::vector<vk::DescriptorSet> m_viewportTextures;
    };
}
