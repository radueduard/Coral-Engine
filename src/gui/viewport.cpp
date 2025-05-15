//
// Created by radue on 5/11/2025.
//

#include "viewport.h"

#include "core/scheduler.h"
#include "ecs/components/camera.h"
#include "project/renderGraph.h"

namespace Coral::Reef {
	Viewport::Viewport(Graphics::RenderPass& renderPass): m_renderPass(renderPass) {
		constexpr auto createInfo = Memory::Sampler::CreateInfo {
			.magFilter = vk::Filter::eLinear,
			.minFilter = vk::Filter::eLinear,
			.addressMode = vk::SamplerAddressMode::eClampToEdge,
			.mipmapMode = vk::SamplerMipmapMode::eNearest,
		};

		m_sampler = std::make_unique<Memory::Sampler>(createInfo);
	}

	void Viewport::OnGUIAttach() {
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

		m_image = new Image(m_viewportTextures[0]);
		AddDockable("viewport", new Reef::Dockable (
            "Main Viewport",
            {
	            .padding = { 10.f, 10.f, 10.f, 10.f },
	            .spacing = 10.f,
	            .backgroundColor = { 0.0f, 0.0f, 0.0f, 1.f },
            },
            {
	            m_image,
            },
            [this] (const Math::Vector2<f32> newSize) {
	            Core::GlobalScheduler().RenderGraph().Resize(newSize, true);
            	ECS::Scene::Get().MainCamera().Resize(newSize);

	            for (uint32_t i = 0; i < m_renderPass.ImageCount(); i++) {
		            ImGui_ImplVulkan_RemoveTexture(m_viewportTextures[i]);
	            }
	            m_viewportTextures.clear();
	            m_viewportTextures.reserve(m_renderPass.ImageCount());
	            const uint32_t outputAttachmentIndex = m_renderPass.OutputAttachmentIndex();
	            for (uint32_t i = 0; i < m_renderPass.ImageCount(); i++) {
		            const auto& framebuffer = m_renderPass.Framebuffer(i);
		            m_viewportTextures.emplace_back(ImGui_ImplVulkan_AddTexture(
			            **m_sampler,
			            *framebuffer.ImageView(outputAttachmentIndex),
			            static_cast<VkImageLayout>(vk::ImageLayout::eShaderReadOnlyOptimal)));
	            }
	            m_image->SetTexture(m_viewportTextures[Core::GlobalScheduler().CurrentFrame().ImageIndex()]);
            }
        ));
	}

	void Viewport::OnGUIDetach() {
		for (uint32_t i = 0; i < m_renderPass.ImageCount(); i++) {
			ImGui_ImplVulkan_RemoveTexture(m_viewportTextures[i]);
		}
	}

	void Viewport::OnGUIUpdate() {
		m_image->SetTexture(m_viewportTextures[Core::GlobalScheduler().CurrentFrame().ImageIndex()]);
	}
}
