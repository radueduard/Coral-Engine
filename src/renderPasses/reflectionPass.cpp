//
// Created by radue on 11/29/2024.
//

#include "reflectionPass.h"

#include "renderer.h"
#include "graphics/renderPass.h"
#include "memory/image.h"

ReflectionPass::ReflectionPass()
    : m_imageCount(mgv::Renderer::SwapChain().ImageCount()), m_extent(512, 512) {
    auto colorAttachment = Graphics::RenderPass::Attachment {
        .description = vk::AttachmentDescription()
            .setFormat(vk::Format::eR8G8B8A8Srgb)
            .setSamples(vk::SampleCountFlagBits::e1)
            .setLoadOp(vk::AttachmentLoadOp::eClear)
            .setStoreOp(vk::AttachmentStoreOp::eStore)
            .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
            .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
            .setInitialLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
            .setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal),
        .reference = vk::AttachmentReference()
            .setAttachment(0)
            .setLayout(vk::ImageLayout::eColorAttachmentOptimal),
        .clearValue = vk::ClearColorValue(std::array { 0.0f, 0.0f, 0.0f, 1.0f })
    };

    auto depthAttachment = Graphics::RenderPass::Attachment {
        .description = vk::AttachmentDescription()
            .setFormat(vk::Format::eD32SfloatS8Uint)
            .setSamples(vk::SampleCountFlagBits::e1)
            .setLoadOp(vk::AttachmentLoadOp::eClear)
            .setStoreOp(vk::AttachmentStoreOp::eDontCare)
            .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
            .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
            .setInitialLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)
            .setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal),
        .reference = vk::AttachmentReference()
            .setAttachment(1)
            .setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal),
        .clearValue = vk::ClearDepthStencilValue(1.0f, 0)
    };

    for (uint32_t i = 0; i < m_imageCount; i++) {
        colorAttachment.images.emplace_back(Memory::Image::Builder()
            .Format(vk::Format::eR8G8B8A8Srgb)
            .Extent({ m_extent.width, m_extent.height, 1 })
            .UsageFlags(vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled)
            .MipLevels(1)
            .LayersCount(1)
            .SampleCount(vk::SampleCountFlagBits::e1)
            .InitialLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
            .Build());

        depthAttachment.images.emplace_back(Memory::Image::Builder()
            .Format(vk::Format::eD32SfloatS8Uint)
            .Extent({ m_extent.width, m_extent.height, 1 })
            .UsageFlags(vk::ImageUsageFlagBits::eDepthStencilAttachment)
            .MipLevels(1)
            .LayersCount(1)
            .SampleCount(vk::SampleCountFlagBits::e1)
            .InitialLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)
            .Build());
    }

    auto colorAttachments = std::array {
        colorAttachment.reference,
    };

    auto subpass = vk::SubpassDescription()
        .setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
        .setColorAttachments(colorAttachments)
        .setPDepthStencilAttachment(&depthAttachment.reference);

    auto dependency1 = vk::SubpassDependency()
        .setSrcSubpass(vk::SubpassExternal)
        .setDstSubpass(0)
        .setSrcStageMask(vk::PipelineStageFlagBits::eTopOfPipe)
        .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests)
        .setSrcAccessMask(vk::AccessFlagBits::eNone)
        .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite);

    auto dependency2 = vk::SubpassDependency()
        .setSrcSubpass(0)
        .setDstSubpass(vk::SubpassExternal)
        .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests)
        .setDstStageMask(vk::PipelineStageFlagBits::eBottomOfPipe)
        .setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite)
        .setDstAccessMask(vk::AccessFlagBits::eNone);

    m_renderPass = Graphics::RenderPass::Builder()
        .ImageCount(m_imageCount)
        .OutputImageIndex(0)
        .Attachment(0, std::move(colorAttachment))
        .Attachment(1, std::move(depthAttachment))
        .Subpass(subpass)
        .Dependency(dependency1)
        .Dependency(dependency2)
        .Extent(m_extent)
        .Build();
}

void ReflectionPass::Run(const vk::CommandBuffer &commandBuffer) const {
    m_renderPass->Begin(commandBuffer, mgv::Renderer::CurrentFrame().imageIndex);
    m_renderPass->Draw(commandBuffer);
    m_renderPass->End(commandBuffer);
}
