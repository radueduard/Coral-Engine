//
// Created by radue on 10/24/2024.
//

#include "renderPass.h"

#include "core/device.h"
#include "memory/image.h"

namespace Graphics {

    void RenderPass::Attachment::Resize(vk::Extent2D extent) const {
        for (auto &image : images) {
            image->Resize({extent.width, extent.height, 1});
        }
    }

    RenderPass::RenderPass(const Core::Device& device, Builder* builder)
        : m_device(device), m_outputAttachmentIndex(builder->m_outputImageIndex), m_imageCount(builder->m_imageCount), m_extent(builder->m_extent) {
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

        m_renderPass = m_device.Handle().createRenderPass(renderPassInfo);
    }

    void RenderPass::CreateFrameBuffers() {
        m_frameBuffers.resize(m_imageCount);
        for (size_t i = 0; i < m_imageCount; i++) {
            auto frameBufferAttachments = std::vector<vk::ImageView>();
            for (const auto &attachment : m_attachments) {
                auto imageView = Memory::ImageView::Builder(*attachment.images[i])
                    .ViewType(vk::ImageViewType::e2D)
                    .BaseMipLevel(0)
                    .LevelCount(1)
                    .Build(m_device);
                frameBufferAttachments.emplace_back(imageView->Handle());
                m_imageViews.emplace_back(std::move(imageView));
            }

            const auto frameBufferInfo = vk::FramebufferCreateInfo()
                .setRenderPass(m_renderPass)
                .setAttachments(frameBufferAttachments)
                .setWidth(m_extent.width)
                .setHeight(m_extent.height)
                .setLayers(1);

            m_frameBuffers[i] = m_device.Handle().createFramebuffer(frameBufferInfo);
        }
    }

    void RenderPass::DestroyRenderPass() {
        if (m_renderPass) {
            m_device.Handle().destroyRenderPass(m_renderPass);
            m_renderPass = nullptr;
        }
    }

    void RenderPass::DestroyFrameBuffers() {
        for (const auto &frameBuffer : m_frameBuffers) {
            m_device.Handle().destroyFramebuffer(frameBuffer);
        }
        m_frameBuffers.clear();

        for (auto &imageView : m_imageViews) {
            imageView.reset();
        }
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
        // for (const auto& program : m_programs) {
        //     if (program) {
        //         program->Update(deltaTime);
        //     }
        // }
    }

    void RenderPass::Draw(const vk::CommandBuffer commandBuffer) const {
        // for (const auto& program : m_programs) {
        //     if (program) {
        //         program->Draw(commandBuffer, this);
        //     }
        // }
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

        m_device.Handle().waitIdle();

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
