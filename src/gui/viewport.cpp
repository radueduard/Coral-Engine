//
// Created by radue on 5/11/2025.
//

#include "viewport.h"

#include "context.h"
#include "core/scheduler.h"
#include "ecs/components/camera.h"
#include "ecs/entity.h"
#include "ecs/sceneManager.h"
#include "gui/elements/popup.h"
#include "project/renderGraph.h"

namespace Coral::Reef {
	Viewport::Viewport(Graphics::RenderPass& renderPass): m_renderPass(renderPass) {
		m_sampler = Memory::Sampler::Builder().Build();
	}

	void Viewport::OnGUIAttach() {
		for (const auto texture : m_viewportTextures) {
			ImGui_ImplVulkan_RemoveTexture(texture);
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

		m_image = new MultiImage(m_viewportTextures | std::views::transform([](const vk::DescriptorSet& ds) {
			return static_cast<ImTextureID>(ds);
		}) | std::ranges::to<std::vector>());

		AddDockable("viewport", new Reef::Window (
            "Main Viewport",
            {
	            .padding = { 10.f, 10.f, 10.f, 10.f },
	            .spacing = 10.f,
	            .backgroundColor = { 0.0f, 0.0f, 0.0f, 1.f },
            },
            {
	            m_image,
            },
            [this] (const Math::Vector2<f32>& newSize) {
	            Context::Scheduler().RenderGraph().Resize(newSize, true);
            	if (ECS::SceneManager::Get().IsSceneLoaded()) {
            		auto& scene = ECS::SceneManager::Get().GetLoadedScene();
					scene.PrimaryCamera().Resize(newSize);

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
					m_image->SetTextures(m_viewportTextures | std::views::transform([](const vk::DescriptorSet& ds) {
							return static_cast<ImTextureID>(ds);
						}) | std::ranges::to<std::vector>(),
						Context::Scheduler().CurrentFrame().ImageIndex());
				}
            }
        ));
	}

	void Viewport::OnGUIDetach() {
		for (uint32_t i = 0; i < m_renderPass.ImageCount(); i++) {
			ImGui_ImplVulkan_RemoveTexture(m_viewportTextures[i]);
		}
	}
}
