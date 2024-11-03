//
// Created by radue on 10/24/2024.
//

#pragma once

#include <iostream>
#include <vector>

#include <core/device.h>
#include <memory/image.h>

namespace Graphics {
    class RenderPass {
    public:
        struct Attachment {
            vk::AttachmentDescription description;
            vk::AttachmentReference reference;
            std::vector<std::unique_ptr<Memory::Image>> images;
            vk::ClearValue clearValue;

            void Resize();
        };

        class Builder {
            friend class RenderPass;
        public:
            Builder() = default;

            Builder& ImageCount(const uint32_t imageCount) {
                m_imageCount = imageCount;
                return *this;
            }

            Builder& OutputImageIndex(const uint32_t outputImageIndex) {
                m_outputImageIndex = outputImageIndex;
                return *this;
            }

            Builder& Extent(const vk::Extent2D extent) {
                m_extent = extent;
                return *this;
            }

            Builder& Attachment(const uint32_t index, Attachment attachment) {
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

            std::unique_ptr<RenderPass> Build(const Core::Device &device) {
                return std::make_unique<RenderPass>(device, *this);
            }


        private:
            uint32_t m_imageCount = 2;
            uint32_t m_outputImageIndex = 0;
            vk::Extent2D m_extent;
            std::vector<RenderPass::Attachment> m_attachments;
            std::vector<vk::SubpassDescription> m_subpasses;
            std::vector<vk::SubpassDependency> m_dependencies;
        };


        explicit RenderPass(const Core::Device &device, Builder &builder);
        ~RenderPass();

        RenderPass(const RenderPass &) = delete;
        RenderPass &operator=(const RenderPass &) = delete;

        void Begin(vk::CommandBuffer commandBuffer, uint32_t imageIndex) const;
        void End(vk::CommandBuffer commandBuffer) const;

        const vk::RenderPass& operator*() const { return m_renderPass; }
        [[nodiscard]] const vk::Framebuffer& FrameBuffer(const uint32_t index) const { return m_frameBuffers[index]; }
        [[nodiscard]] const vk::Extent2D& Extent() const { return m_extent; }

        const std::unique_ptr<Memory::Image>& OutputImage(uint32_t index);

        void CreateRenderPass();
        void DestroyRenderPass();
        void CreateFrameBuffers();
        void DestroyFrameBuffers();

        bool Resize(uint32_t imageCount, vk::Extent2D extent);

    private:
        const Core::Device &m_device;
        uint32_t m_outputImageIndex = 0;
        uint32_t m_imageCount;
        vk::Extent2D m_extent;

        vk::RenderPass m_renderPass;
        std::vector<vk::Framebuffer> m_frameBuffers;

        std::vector<Attachment> m_attachments;
        std::vector<vk::SubpassDescription> m_subpasses;
        std::vector<vk::SubpassDependency> m_dependencies;
    };
}
