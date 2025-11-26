//
// Created by radue on 10/24/2024.
//

#include "renderPass.h"

#include <ranges>

#include "core/device.h"
#include "ecs/components/camera.h"
#include "ecs/components/renderTarget.h"
#include "ecs/components/transform.h"
#include "ecs/scene.h"
#include "ecs/sceneManager.h"
#include "ecs/entity.h"

#include "framebuffer.h"
#include "memory/image.h"

#include "gui/elements/popup.h"

namespace Coral::Graphics {
    void RenderPass::Attachment::Resize(const Math::Vector2<f32>& extent) const {
        for (auto* image : images) {
            image->Resize(Math::Vector3u { static_cast<u32>(extent.x), static_cast<u32>(extent.y), 1u });
        }
    }

    RenderPass::RenderPass(Builder* builder)
        : m_outputAttachmentIndex(builder->m_outputImageIndex),
		m_imageCount(builder->m_imageCount),
		m_extent(builder->m_extent) {
        m_attachments = std::move(builder->m_attachments);
        m_subpasses = std::move(builder->m_subpasses);
        m_dependencies = builder->m_dependencies;
        m_sampleCount = m_attachments[0].description.samples;

        CreateRenderPass();
        CreateFrameBuffers();
    }

    RenderPass::~RenderPass() {
        DestroyFrameBuffers();
        DestroyRenderPass();
    }

    void RenderPass::CreateRenderPass() {
        std::vector<vk::AttachmentDescription> attachmentDescriptions;
        attachmentDescriptions.reserve(m_attachments.size());
        for (const auto &attachment : m_attachments) {
            attachmentDescriptions.emplace_back(attachment.description);
        }

    	std::vector<vk::SubpassDescription> subpassDescriptions;
    	subpassDescriptions.reserve(m_subpasses.size());
    	for (const auto &subpass : m_subpasses) {
    		auto subpassDescription = vk::SubpassDescription();
    		if (!subpass.colorAttachments.empty()) {
				subpassDescription.setColorAttachments(subpass.colorAttachments);
			}
    		if (!subpass.inputAttachments.empty()) {
				subpassDescription.setInputAttachments(subpass.inputAttachments);
			}
    		if (!subpass.resolveAttachments.empty()) {
    			subpassDescription.setResolveAttachments(subpass.resolveAttachments);
    		}
    		if (subpass.depthStencilAttachment.has_value()) {
    			subpassDescription.setPDepthStencilAttachment(&subpass.depthStencilAttachment.value());
    		}
			subpassDescriptions.emplace_back(subpassDescription);
		}

		if (m_outputAttachmentIndex >= m_attachments.size()) {
			throw std::runtime_error("Output attachment index is out of bounds.");
		}

        const auto renderPassInfo = vk::RenderPassCreateInfo()
            .setAttachments(attachmentDescriptions)
            .setSubpasses(subpassDescriptions)
            .setDependencies(m_dependencies);

        m_handle = Context::Device()->createRenderPass(renderPassInfo);
    }

    void RenderPass::CreateFrameBuffers() {
        m_frameBuffers.resize(m_imageCount);
        for (u32 i = 0; i < m_imageCount; i++) {
            m_frameBuffers[i] = std::make_unique<Graphics::Framebuffer>(*this, i);
        }
    }

    void RenderPass::DestroyRenderPass() {
        if (m_handle) {
            Context::Device()->destroyRenderPass(m_handle);
            m_handle = nullptr;
        }
    }

    void RenderPass::DestroyFrameBuffers() {
        m_frameBuffers.clear();

        for (auto &imageView : m_imageViews) {
            imageView.reset();
        }
    }

    void RenderPass::Begin(const Core::CommandBuffer& commandBuffer, const u32 imageIndex) {
        m_inFlightImageIndex = imageIndex;

        auto clearValues = m_attachments
            | std::views::transform([](const auto& attachment) { return attachment.clearValue; })
            | std::ranges::to<std::vector>();

        const auto renderPassInfo = vk::RenderPassBeginInfo()
            .setRenderPass(m_handle)
            .setFramebuffer(**m_frameBuffers[imageIndex])
            .setRenderArea(vk::Rect2D().setExtent({ static_cast<u32>(m_extent.x), static_cast<u32>(m_extent.y) }))
            .setClearValues(clearValues);

        commandBuffer->beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

        const auto viewport = vk::Viewport()
            .setX(0.0f)
            .setY(0.0f)
            .setWidth(m_extent.x)
            .setHeight(m_extent.y)
            .setMinDepth(0.0f)
            .setMaxDepth(1.0f);

        const auto scissor = vk::Rect2D()
            .setOffset({0, 0})
            .setExtent({ static_cast<u32>(m_extent.x), static_cast<u32>(m_extent.y) });

        commandBuffer->setViewport(0, viewport);
        commandBuffer->setScissor(0, scissor);
    }

    void RenderPass::Update(const float deltaTime) {
        for (auto& [builder, pipeline] : m_pipelines) {
        	bool needsUpdate = builder->ShouldRebuild();
        	for (const auto& shader : pipeline->Shaders() | std::views::values) {
				needsUpdate |= shader->HasReloaded();
			}
        	if (needsUpdate) {
        		if (std::ranges::all_of(pipeline->Shaders() | std::views::values, [](const Shader::Shader* shader) { return true; })) {
        			builder->m_shaders = std::move(pipeline->m_shaders);
        			pipeline = builder->Build();
        		}
        	}
        }
    }

    void RenderPass::Draw(const Core::CommandBuffer& commandBuffer) const {
		if (!ECS::SceneManager::Get().IsSceneLoaded())
			return;

    	for (const auto& pipeline : m_pipelines | std::views::values) {
            pipeline->Bind(*commandBuffer);
            pipeline->BindDescriptorSet(0, *commandBuffer, ECS::SceneManager::Get().GetLoadedScene().DescriptorSet());
            ECS::SceneManager::Get().Registry().group(entt::get<ECS::Entity*, ECS::RenderTarget>).each(
            [&](const ECS::Entity* entity, const ECS::RenderTarget& renderTarget) {
                Math::Matrix4<f32> matrix = Math::Matrix4<f32>::Identity();
            	while (entity) {
            		auto& transform = entity->Get<ECS::Transform>();
            		matrix *= transform.Matrix();
            		entity = entity->Parent();
            	}
                for (const auto [mesh, material] : renderTarget.Targets()) {
                	// pipeline->BindDescriptorSet(1, *commandBuffer, material->DescriptorSet());
                    pipeline->PushConstants<Math::Matrix4<f32>>(*commandBuffer, vk::ShaderStageFlagBits::eTessellationEvaluation, 0, matrix);
                    mesh->Bind(*commandBuffer);
                    mesh->Draw(*commandBuffer);
                }
            });
        }
    }

    void RenderPass::End(const Core::CommandBuffer& commandBuffer)  {
        commandBuffer->endRenderPass();
        m_inFlightImageIndex = std::nullopt;
    }

    Memory::Image& RenderPass::OutputImage(const uint32_t index) const {
        return *m_attachments[m_outputAttachmentIndex].images[index];
    }

    Memory::Image& RenderPass::CurrentOutputImage() const {
        return OutputImage(m_outputImageIndex);
    }


    bool RenderPass::Resize(const u32 imageCount, const Math::Vector2<f32>& extent) {
        if ((m_imageCount == imageCount && m_extent == extent) || (extent.x == 0 || extent.y == 0)) {
            return false;
        }

        Context::Device()->waitIdle();

        DestroyFrameBuffers();

        m_imageCount = imageCount;
        m_extent = extent;

        for (auto &attachment : m_attachments) {
            attachment.Resize(extent);
        }
        CreateFrameBuffers();
        return true;
    }
}
