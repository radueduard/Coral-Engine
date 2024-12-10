//
// Created by radue on 12/6/2024.
//

#include "depthPrepass.h"

#include "renderer.h"
#include "graphics/renderPass.h"
#include "memory/image.h"

DepthPrepass::DepthPrepass()
    : m_imageCount(mgv::Renderer::ImageCount()), m_extent(512, 512) {
    auto depthAttachment = Graphics::RenderPass::Attachment {
        .description = vk::AttachmentDescription()
            .setFormat(vk::Format::eD32Sfloat)
            .setSamples(vk::SampleCountFlagBits::e1)
            .setLoadOp(vk::AttachmentLoadOp::eClear)
            .setStoreOp(vk::AttachmentStoreOp::eDontCare)
            .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
            .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
            .setInitialLayout(vk::ImageLayout::eDepthStencilReadOnlyOptimal)
            .setFinalLayout(vk::ImageLayout::eDepthStencilReadOnlyOptimal),
        .reference = vk::AttachmentReference()
            .setAttachment(0)
            .setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal),
        .clearValue = vk::ClearDepthStencilValue(1.0f, 0)
    };

    for (uint32_t i = 0; i < m_imageCount; i++) {
        depthAttachment.images.emplace_back(Memory::Image::Builder()
            .Format(vk::Format::eD32Sfloat)
            .Extent({ m_extent.width, m_extent.height, 1 })
            .UsageFlags(vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled)
            .MipLevels(1)
            .LayersCount(1)
            .SampleCount(vk::SampleCountFlagBits::e1)
            .InitialLayout(vk::ImageLayout::eDepthReadOnlyOptimal)
            .Build());
    }

    const auto subpass = vk::SubpassDescription()
        .setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
        .setPDepthStencilAttachment(&depthAttachment.reference);

    const auto dependency1 = vk::SubpassDependency()
        .setSrcSubpass(vk::SubpassExternal)
        .setDstSubpass(0)
        .setSrcStageMask(vk::PipelineStageFlagBits::eTopOfPipe)
        .setDstStageMask(vk::PipelineStageFlagBits::eEarlyFragmentTests)
        .setSrcAccessMask(vk::AccessFlagBits::eNone)
        .setDstAccessMask(vk::AccessFlagBits::eDepthStencilAttachmentWrite);

    const auto dependency2 = vk::SubpassDependency()
        .setSrcSubpass(0)
        .setDstSubpass(vk::SubpassExternal)
        .setSrcStageMask(vk::PipelineStageFlagBits::eEarlyFragmentTests)
        .setDstStageMask(vk::PipelineStageFlagBits::eBottomOfPipe)
        .setSrcAccessMask(vk::AccessFlagBits::eDepthStencilAttachmentWrite)
        .setDstAccessMask(vk::AccessFlagBits::eNone);

    m_renderPass = Graphics::RenderPass::Builder()
        .ImageCount(m_imageCount)
        .OutputImageIndex(0)
        .Attachment(0, std::move(depthAttachment))
        .Subpass(subpass)
        .Dependency(dependency1)
        .Dependency(dependency2)
        .Extent(m_extent)
        .Build();
}

void DepthPrepass::Run(const vk::CommandBuffer &commandBuffer) const {
    m_renderPass->Begin(commandBuffer, mgv::Renderer::CurrentFrame().imageIndex);
    m_renderPass->Draw(commandBuffer);
    m_renderPass->End(commandBuffer);
}
