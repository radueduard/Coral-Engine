//
// Created by radue on 10/24/2024.
//

#include "renderPass.h"

namespace Graphics {

    void RenderPass::Attachment::Resize(vk::Extent2D extent) const {
        for (auto &image : images) {
            image->Resize({extent.width, extent.height, 1});
        }
    }

    RenderPass::RenderPass(Builder* builder)
        : m_outputAttachmentIndex(builder->m_outputImageIndex), m_imageCount(builder->m_imageCount), m_extent(builder->m_extent) {
        m_attachments = std::move(builder->m_attachments);
        m_subpasses = builder->m_subpasses;
        m_dependencies = builder->m_dependencies;
        m_sampleCount = m_attachments[0].description.samples;
        m_programs.resize(m_subpasses.size());

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

        m_renderPass = (*Core::Device::Get()).createRenderPass(renderPassInfo);
    }

    void RenderPass::CreateFrameBuffers() {
        m_frameBuffers.resize(m_imageCount);
        for (size_t i = 0; i < m_imageCount; i++) {
            auto frameBufferAttachments = std::vector<vk::ImageView>();
            for (const auto &attachment : m_attachments) {
                frameBufferAttachments.emplace_back(attachment.images[i]->ImageView());
            }

            const auto frameBufferInfo = vk::FramebufferCreateInfo()
                .setRenderPass(m_renderPass)
                .setAttachments(frameBufferAttachments)
                .setWidth(m_extent.width)
                .setHeight(m_extent.height)
                .setLayers(1);

            m_frameBuffers[i] = (*Core::Device::Get()).createFramebuffer(frameBufferInfo);
        }
    }

    void RenderPass::DestroyRenderPass() {
        if (m_renderPass) {
            (*Core::Device::Get()).destroyRenderPass(m_renderPass);
            m_renderPass = nullptr;
        }
    }

    void RenderPass::DestroyFrameBuffers() {
        for (const auto &frameBuffer : m_frameBuffers) {
            (*Core::Device::Get()).destroyFramebuffer(frameBuffer);
        }
        m_frameBuffers.clear();
    }

    void RenderPass::Begin(const vk::CommandBuffer commandBuffer, const uint32_t imageIndex) {
        m_inFlightImageIndex = imageIndex;
        auto clearValues = std::vector<vk::ClearValue>();
        for (const auto &attachment : m_attachments) {
            clearValues.emplace_back(attachment.clearValue);
        }

        const auto renderPassInfo = vk::RenderPassBeginInfo()
            .setRenderPass(m_renderPass)
            .setFramebuffer(m_frameBuffers[imageIndex])
            .setRenderArea(vk::Rect2D().setExtent(m_extent))
            .setClearValues(clearValues);

        commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

        const auto viewport = vk::Viewport()
            .setX(0.0f)
            .setY(0.0f)
            .setWidth(static_cast<float>(m_extent.width))
            .setHeight(static_cast<float>(m_extent.height))
            .setMinDepth(0.0f)
            .setMaxDepth(1.0f);

        const auto scissor = vk::Rect2D()
            .setOffset({0, 0})
            .setExtent(m_extent);

        commandBuffer.setViewport(0, viewport);
        commandBuffer.setScissor(0, scissor);
    }

    void RenderPass::Update(const float deltaTime) const {
        for (const auto& subpass : m_programs) {
            for (const auto& program : subpass) {
                program->Update(deltaTime);
            }
        }
    }

    void RenderPass::Draw(const vk::CommandBuffer commandBuffer, const bool reflected) const {
        uint32_t i = 0;
        for (const auto& subpass : m_programs) {
            for (const auto& program : subpass) {
                program->Draw(commandBuffer, reflected);
            }
            if (i + 1 < m_programs.size()) {
                commandBuffer.nextSubpass(vk::SubpassContents::eInline);
            }
            i++;
        }
    }

    void RenderPass::End(const vk::CommandBuffer commandBuffer)  {
        commandBuffer.endRenderPass();
        m_inFlightImageIndex = std::nullopt;
    }

    Memory::Image& RenderPass::OutputImage(const uint32_t index) const {
        return *m_attachments[m_outputAttachmentIndex].images[index];
    }

    Memory::Image& RenderPass::CurrentOutputImage() const {
        return OutputImage(m_outputImageIndex);
    }


    bool RenderPass::Resize(const uint32_t imageCount, const vk::Extent2D extent) {
        if ((m_imageCount == imageCount && m_extent == extent) || (extent.width == 0 || extent.height == 0)) {
            return false;
        }

        (*Core::Device::Get()).waitIdle();

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
