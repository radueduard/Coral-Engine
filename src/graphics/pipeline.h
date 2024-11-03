//
// Created by radue on 10/17/2024.
//

#pragma once

#include <unordered_map>
#include <unordered_set>

#include "../core/device.h"
#include "../core/shader.h"
#include "memory/descriptor/pool.h"
#include "memory/descriptor/set.h"

namespace Graphics {
    class Pipeline {
        enum Type {
            VTG,
            TM,
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

            Builder &RenderPass(const vk::RenderPass &);
            Builder &Subpass(uint32_t);

            Builder &BasePipeline(const vk::Pipeline &, int32_t);

            template<typename T>
            Builder &PushConstantRange(const vk::ShaderStageFlags &stageFlags, const uint32_t offset = 0) {
                const auto pushConstantRange = vk::PushConstantRange()
                    .setStageFlags(stageFlags)
                    .setOffset(offset)
                    .setSize(sizeof(T));
                m_pushConstantRanges.push_back(pushConstantRange);
                return *this;
            }

            Builder &DescriptorSetLayout(uint32_t setNumber, const Memory::Descriptor::SetLayout &layout);
            Builder &DescriptorSetLayouts(uint32_t startingSet, const std::vector<Memory::Descriptor::SetLayout> &layouts);

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

            std::vector<vk::PushConstantRange> m_pushConstantRanges;
            std::vector<vk::DescriptorSetLayout> m_descriptorSetLayouts;
        };

        Pipeline(const Pipeline &) = delete;
        Pipeline &operator=(const Pipeline &) = delete;

        void Bind(const vk::CommandBuffer&) const;

        Pipeline(const Core::Device &, const Builder &);
        ~Pipeline();

        void BindDescriptorSet(uint32_t, vk::CommandBuffer, const Memory::Descriptor::Set &) const;
        void BindDescriptorSets(uint32_t, vk::CommandBuffer, const std::vector<Memory::Descriptor::Set> &) const;

        template<typename T>
        void PushConstants(const vk::CommandBuffer commandBuffer, const vk::ShaderStageFlags stageFlags, const uint32_t offset, const T& data) const {
            commandBuffer.pushConstants(
                m_pipelineLayout,
                stageFlags,
                offset,
                sizeof(T),
                &data);
        }
    private:

        const Core::Device &m_device;
        Type m_type;
        vk::Pipeline m_pipeline;
        vk::PipelineLayout m_pipelineLayout;
        std::unordered_map<vk::ShaderStageFlagBits, vk::ShaderModule> m_shaders;
    };
}
