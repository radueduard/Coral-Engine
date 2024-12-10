//
// Created by radue on 10/24/2024.
//

#pragma once

#include <iostream>
#include <optional>
#include <vector>

#include <vulkan/vulkan.hpp>

#include "memory/image.h"

namespace Graphics {
    class Program;
}

namespace Graphics {
    class RenderPass {
    public:
        struct Attachment {
            vk::AttachmentDescription description;
            vk::AttachmentReference reference;
            std::vector<std::unique_ptr<Memory::Image>> images;
            vk::ClearValue clearValue;

            void Resize(vk::Extent2D extent) const;
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

            std::unique_ptr<RenderPass> Build() {
                return std::make_unique<RenderPass>(this);
            }


        private:
            uint32_t m_imageCount = 2;
            uint32_t m_outputImageIndex = 0;
            vk::Extent2D m_extent;
            std::vector<RenderPass::Attachment> m_attachments;
            std::vector<vk::SubpassDescription> m_subpasses;
            std::vector<vk::SubpassDependency> m_dependencies;
        };


        explicit RenderPass(Builder *builder);
        ~RenderPass();

        RenderPass(const RenderPass &) = delete;
        RenderPass &operator=(const RenderPass &) = delete;

        void AddProgram(Program *program) { m_programs.emplace_back(program); }
        void RemoveProgram(Program *program) { std::erase(m_programs, program); }

        void Begin(vk::CommandBuffer commandBuffer, uint32_t imageIndex);
        void Update(float deltaTime) const;
        void Draw(vk::CommandBuffer commandBuffer) const;
        void End(vk::CommandBuffer commandBuffer);

        const vk::RenderPass& operator*() const { return m_renderPass; }
        [[nodiscard]] const vk::Framebuffer& FrameBuffer(const uint32_t index) const { return m_frameBuffers[index]; }
        [[nodiscard]] const vk::Extent2D& Extent() const { return m_extent; }
        [[nodiscard]] const vk::SampleCountFlagBits& SampleCount() const { return m_sampleCount; }
        [[nodiscard]] uint32_t OutputImageIndex() const { return m_outputImageIndex; }
        [[nodiscard]] uint32_t InFlightImageIndex() const {
            if (!m_inFlightImageIndex.has_value()) {
                std::cerr << "No in flight image index" << std::endl;
                return -1;
            }
            return m_inFlightImageIndex.value();
        }

        [[nodiscard]] Memory::Image& OutputImage(uint32_t index) const;
        [[nodiscard]] Memory::Image& CurrentOutputImage() const;

        [[nodiscard]] std::vector<Program*>& Programs() { return m_programs; }

        void CreateRenderPass();
        void DestroyRenderPass();
        void CreateFrameBuffers();
        void DestroyFrameBuffers();

        bool Resize(uint32_t imageCount, vk::Extent2D extent);

    private:
        uint32_t m_outputAttachmentIndex = 0;
        uint32_t m_outputImageIndex = 0;
        std::optional<uint32_t> m_inFlightImageIndex;
        uint32_t m_imageCount;
        vk::Extent2D m_extent;

        vk::RenderPass m_renderPass;
        std::vector<vk::Framebuffer> m_frameBuffers;

        std::vector<Attachment> m_attachments;
        std::vector<vk::SubpassDescription> m_subpasses;
        std::vector<vk::SubpassDependency> m_dependencies;

        vk::SampleCountFlagBits m_sampleCount = vk::SampleCountFlagBits::e1;
        std::vector<Program*> m_programs;
    };
}
