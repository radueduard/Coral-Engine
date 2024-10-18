//
// Created by radue on 10/17/2024.
//

#pragma once

#include "pipeline.h"

namespace Graphics {
    class Program {
    public:
        Program(const Core::Device &device, const vk::RenderPass& renderPass, const int32_t subpass);
        ~Program();

        void BindPipeline(const vk::CommandBuffer& commandBuffer) const;

        Program(const Program &) = delete;
        Program &operator=(const Program &) = delete;
    private:
        const Core::Device &m_device;
        std::unique_ptr<Graphics::Pipeline> m_pipeline;
        vk::PipelineLayout m_layout;
    };
}