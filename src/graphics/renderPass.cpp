//
// Created by radue on 10/24/2024.
//

#include "renderPass.h"

#include <ranges>

#include "framebuffer.h"
#include "core/device.h"
#include "memory/image.h"

namespace Graphics {
    void RenderPass::Attachment::Resize(const Math::Vector2<uint32_t>& extent) const {
        for (auto* image : images) {
            image->Resize({extent.x, extent.y, 1});
        }
    }

    RenderPass::RenderPass(Builder* builder)
        : m_outputAttachmentIndex(builder->m_outputImageIndex), m_imageCount(builder->m_imageCount), m_extent(builder->m_extent) {
        m_attachments = std::move(builder->m_attachments);
        m_subpasses = builder->m_subpasses;
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

        const auto renderPassInfo = vk::RenderPassCreateInfo()
            .setAttachments(attachmentDescriptions)
            .setSubpasses(m_subpasses)
            .setDependencies(m_dependencies);

        m_handle = Core::GlobalDevice()->createRenderPass(renderPassInfo);
    }

    void RenderPass::CreateFrameBuffers() {
        m_frameBuffers.resize(m_imageCount);
        for (uint32_t i = 0; i < m_imageCount; i++) {
            m_frameBuffers[i] = std::make_unique<Graphics::Framebuffer>(*this, i);
        }
    }

    void RenderPass::DestroyRenderPass() {
        if (m_handle) {
            Core::GlobalDevice()->destroyRenderPass(m_handle);
            m_handle = nullptr;
        }
    }

    void RenderPass::DestroyFrameBuffers() {
        m_frameBuffers.clear();

        for (auto &imageView : m_imageViews) {
            imageView.reset();
        }
    }

    void RenderPass::Begin(const Core::CommandBuffer& commandBuffer, const uint32_t imageIndex) {
        m_inFlightImageIndex = imageIndex;

        auto clearValues = m_attachments
            | std::views::transform([](const auto& attachment) { return attachment.clearValue; })
            | std::ranges::to<std::vector>();

        const auto renderPassInfo = vk::RenderPassBeginInfo()
            .setRenderPass(m_handle)
            .setFramebuffer(**m_frameBuffers[imageIndex])
            .setRenderArea(vk::Rect2D().setExtent(m_extent))
            .setClearValues(clearValues);

        commandBuffer->beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

        const auto viewport = vk::Viewport()
            .setX(0.0f)
            .setY(0.0f)
            .setWidth(static_cast<float>(m_extent.x))
            .setHeight(static_cast<float>(m_extent.y))
            .setMinDepth(0.0f)
            .setMaxDepth(1.0f);

        const auto scissor = vk::Rect2D()
            .setOffset({0, 0})
            .setExtent(m_extent);

        commandBuffer->setViewport(0, viewport);
        commandBuffer->setScissor(0, scissor);
    }

    void RenderPass::Update(const float deltaTime) const {
        // for (const auto& program : m_programs) {
        //     if (program) {
        //         program->Update(deltaTime);
        //     }
        // }
    }

    void RenderPass::Draw(const Core::CommandBuffer& commandBuffer) const {
        for (const auto& pipeline : m_pipelines) {
            pipeline->Bind(*commandBuffer);
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


    bool RenderPass::Resize(const uint32_t imageCount, const Math::Vector2<uint32_t>& extent) {
        if ((m_imageCount == imageCount && m_extent == extent) || (extent.x == 0 || extent.y == 0)) {
            return false;
        }

        Core::GlobalDevice()->waitIdle();

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
