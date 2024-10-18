//
// Created by radue on 10/17/2024.
//

#pragma once

#include <unordered_map>
#include <unordered_set>

#include "../core/device.h"
#include "../core/shader.h"

namespace Graphics {
    class Pipeline {
        enum Type {
            VTG,
            TM,
            RT
        };
    public:
        class Builder {
            friend class Pipeline;
        public:
            Builder();
            ~Builder() = default;

            Builder(const Builder &) = delete;
            Builder &operator=(const Builder &) = delete;

            Builder &AddShader(const Core::Shader &);
            Builder &VertexInputState(const vk::PipelineVertexInputStateCreateInfo &);
            Builder &InputAssemblyState(const vk::PipelineInputAssemblyStateCreateInfo &);
            Builder &Viewport(const vk::Viewport &);
            Builder &Scissor(const vk::Rect2D &);
            Builder &Rasterizer(const vk::PipelineRasterizationStateCreateInfo &);
            Builder &Multisampling(const vk::PipelineMultisampleStateCreateInfo &);
            Builder &DepthStencil(const vk::PipelineDepthStencilStateCreateInfo &);

            Builder &ColorBlendAttachment(const vk::PipelineColorBlendAttachmentState &);
            Builder &ColorBlend(const vk::PipelineColorBlendStateCreateInfo &);

            Builder &DynamicState(const vk::DynamicState &);
            Builder &Tessellation(const vk::PipelineTessellationStateCreateInfo &);

            Builder &PipelineLayout(const vk::PipelineLayout &);
            Builder &RenderPass(const vk::RenderPass &);
            Builder &Subpass(uint32_t);

            Builder &BasePipeline(const vk::Pipeline &, int32_t);

            std::unique_ptr<Pipeline> Build(const Core::Device &);
        private:
            void CheckShaderStagesValidity() const;

            std::unordered_map<vk::ShaderStageFlagBits, vk::ShaderModule> m_shaders;
            std::unordered_set<Type> m_yetPossibleTypes;

            std::vector<vk::PipelineShaderStageCreateInfo> m_stages;

            vk::PipelineVertexInputStateCreateInfo m_vertexInputInfo;
            vk::PipelineInputAssemblyStateCreateInfo m_inputAssembly;

            std::vector<vk::Viewport> m_viewports;
            std::vector<vk::Rect2D> m_scissors;
            vk::PipelineViewportStateCreateInfo m_viewportState;

            vk::PipelineRasterizationStateCreateInfo m_rasterizer;
            vk::PipelineDepthStencilStateCreateInfo m_depthStencil;

            std::vector<vk::PipelineColorBlendAttachmentState> m_colorBlendAttachments;
            vk::PipelineColorBlendStateCreateInfo m_colorBlending;
            vk::PipelineMultisampleStateCreateInfo m_multisampling;
            vk::PipelineTessellationStateCreateInfo m_tessellation;

            std::vector <vk::DynamicState> m_dynamicStates = {
                vk::DynamicState::eViewport,
                vk::DynamicState::eScissor
            };
            vk::PipelineDynamicStateCreateInfo m_dynamicState;

            vk::PipelineLayout m_pipelineLayout;
            vk::RenderPass m_renderPass;
            uint32_t m_subpass = 0;
            vk::Pipeline m_basePipeline = nullptr;
            int32_t m_basePipelineIndex = -1;
        };

        Pipeline(const Pipeline &) = delete;
        Pipeline &operator=(const Pipeline &) = delete;

        void Bind(const vk::CommandBuffer&) const;

        Pipeline(const Core::Device &, const Builder &);
        ~Pipeline();
    private:

        const Core::Device &m_device;
        Type m_type;
        vk::Pipeline m_pipeline;
        std::unordered_map<vk::ShaderStageFlagBits, vk::ShaderModule> m_shaders;
    };
}
