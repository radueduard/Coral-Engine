//
// Created by radue on 10/24/2024.
//

#pragma once

#include <vulkan/vulkan.hpp>

#include "pipeline.h"
#include "core/device.h"
#include "memory/image.h"
#include "memory/imageView.h"

import types;
import math.vector;
import std;

using namespace Coral;

namespace Graphics {
    class Framebuffer;

    class RenderPass final : public EngineWrapper<vk::RenderPass> {
    public:
        struct Attachment {
            vk::AttachmentDescription description;
            vk::AttachmentReference reference;
            std::vector<Memory::Image*> images;
            vk::ClearValue clearValue;

            void Resize(const Math::Vector2<u32>& extent) const;
        };

        class Builder {
            friend class RenderPass;
        public:
            Builder() = default;

            Builder& ImageCount(const u32 imageCount) {
                m_imageCount = imageCount;
                return *this;
            }

            Builder& OutputImageIndex(const u32 outputImageIndex) {
                m_outputImageIndex = outputImageIndex;
                return *this;
            }

            Builder& Extent(const Math::Vector2<u32> extent) {
                m_extent = extent;
                return *this;
            }

            Builder& Attachment(const u32 index, Attachment attachment) {
                if (index > m_attachments.size()) {
                    std::cerr << "Attachments should be provided in order" << std::endl;
                    return *this;
                }
                m_attachments.emplace_back(std::move(attachment));
                return *this;
            }

            Builder& Subpass(const vk::SubpassDescription& subpass) {
                m_subpasses.emplace_back(subpass);
                return *this;
            }

            Builder& Dependency(const vk::SubpassDependency& dependency) {
                m_dependencies.emplace_back(dependency);
                return *this;
            }

            std::unique_ptr<RenderPass> Build() {
                return std::make_unique<RenderPass>(this);
            }


        private:
            u32 m_imageCount = 2;
            u32 m_outputImageIndex = 0;
            Math::Vector2<u32> m_extent;
            std::vector<RenderPass::Attachment> m_attachments;
            std::vector<vk::SubpassDescription> m_subpasses;
            std::vector<vk::SubpassDependency> m_dependencies;
        };


        explicit RenderPass(Builder *builder);
        ~RenderPass() override;

        RenderPass(const RenderPass &) = delete;
        RenderPass &operator=(const RenderPass &) = delete;

        void Begin(const Core::CommandBuffer& commandBuffer, u32 imageIndex);
        void Update(float deltaTime) const;
        void Draw(const Core::CommandBuffer& commandBuffer) const;
        void End(const Core::CommandBuffer& commandBuffer);

        [[nodiscard]] const std::vector<Attachment>& Attachments() const { return m_attachments; }
        [[nodiscard]] std::vector<Attachment> Subpass(const u32 index) const {
            if (index >= m_subpasses.size()) {
                std::cerr << "Subpass index out of range" << std::endl;
                return {};
            }
            std::vector<Attachment> attachments;
            for (u32 i = 0; i < m_subpasses[index].colorAttachmentCount; i++) {
                const auto& attachment = m_attachments[m_subpasses[index].pColorAttachments[i].attachment];
                attachments.emplace_back(attachment);
            }
            return attachments;
        }

        [[nodiscard]] const Framebuffer& Framebuffer(const u32 index) const { return *m_frameBuffers[index]; }
        [[nodiscard]] const Math::Vector2<u32>& Extent() const { return m_extent; }
        [[nodiscard]] const vk::SampleCountFlagBits& SampleCount() const { return m_sampleCount; }
        [[nodiscard]] u32 OutputImageIndex() const { return m_outputImageIndex; }
        [[nodiscard]] u32 ImageCount() const { return m_imageCount; }
        [[nodiscard]] u32 InFlightImageIndex() const {
            if (!m_inFlightImageIndex.has_value()) {
                std::cerr << "No in flight image index" << std::endl;
                return -1;
            }
            return m_inFlightImageIndex.value();
        }

        [[nodiscard]] Memory::Image& OutputImage(u32 index) const;
        [[nodiscard]] Memory::Image& CurrentOutputImage() const;

        void CreateRenderPass();
        void DestroyRenderPass();
        void CreateFrameBuffers();
        void DestroyFrameBuffers();

        void AddPipeline(std::unique_ptr<Pipeline> pipeline) {
            m_pipelines.emplace_back(std::move(pipeline));
        }

        bool Resize(u32 imageCount, const Math::Vector2<u32>& extent);

    private:
        u32 m_outputAttachmentIndex = 0;
        u32 m_outputImageIndex = 0;
        std::optional<u32> m_inFlightImageIndex;
        u32 m_imageCount;
        Math::Vector2<u32> m_extent;

        std::vector<std::unique_ptr<Graphics::Framebuffer>> m_frameBuffers;

        std::vector<Attachment> m_attachments;
        std::vector<std::unique_ptr<Memory::ImageView>> m_imageViews;

        std::vector<vk::SubpassDescription> m_subpasses;
        std::vector<vk::SubpassDependency> m_dependencies;

        vk::SampleCountFlagBits m_sampleCount = vk::SampleCountFlagBits::e1;

        std::vector<std::unique_ptr<Pipeline>> m_pipelines;
    };
}
