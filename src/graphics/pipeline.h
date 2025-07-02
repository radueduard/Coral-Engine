//
// Created by radue on 10/17/2024.
//

#pragma once

#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <vulkan/vulkan.hpp>

#include "../shader/shader.h"
#include "memory/descriptor/set.h"
#include "objects/mesh.h"

namespace Coral::Memory::Descriptor {
    class SetLayout;
}

namespace Coral::Graphics {
    class RenderPass;

    class Pipeline {
    	friend class RenderPass;
    public:
        class Builder {
            friend class Pipeline;
        	friend class RenderPass;
        public:
            Builder(RenderPass&);
            ~Builder() = default;

            Builder(const Builder &) = delete;
            Builder &operator=(const Builder &) = delete;

            Builder &AddShader(const Core::Shader* shader);
            Builder &InputAssemblyState(const vk::PipelineInputAssemblyStateCreateInfo &);
            Builder &Viewport(const vk::Viewport &);
            Builder &Scissor(const vk::Rect2D &);
            Builder &Rasterizer(const vk::PipelineRasterizationStateCreateInfo &);
            Builder &DepthStencil(const vk::PipelineDepthStencilStateCreateInfo &);

            Builder &DynamicState(const vk::DynamicState &);
            Builder &Tessellation(const vk::PipelineTessellationStateCreateInfo &);

            Builder &Subpass(uint32_t);

        	Builder &BindFunction(const std::function<void(const vk::CommandBuffer&, const Mesh&)> &function);

            std::unique_ptr<Pipeline> Build();
        private:
			RenderPass &m_renderPass;

            std::vector<std::unique_ptr<Memory::Descriptor::SetLayout>> m_setLayouts;
            std::unordered_map<Core::Stage, const Core::Shader*> m_shaders;
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
            uint32_t m_subpass = 0;
        };

        explicit Pipeline(Builder &);
        ~Pipeline();

        Pipeline(const Pipeline &) = delete;
        Pipeline &operator=(const Pipeline &) = delete;

        void Bind(const vk::CommandBuffer&) const;
        template<typename T>
        void PushConstants(const vk::CommandBuffer commandBuffer, const vk::ShaderStageFlags stageFlags, const uint32_t offset, const T& data) const {
            commandBuffer.pushConstants(
                m_pipelineLayout,
                stageFlags,
                offset,
                sizeof(T),
                &data);
        }

        void BindDescriptorSet(uint32_t, vk::CommandBuffer, const Memory::Descriptor::Set &) const;
        void BindDescriptorSets(uint32_t, vk::CommandBuffer, const std::vector<Memory::Descriptor::Set> &) const;

        [[nodiscard]] const vk::PipelineLayout &Layout() const { return m_pipelineLayout; }

        const std::unordered_map<Core::Stage, const Core::Shader*>& Shaders() { return m_shaders; }
    private:
        vk::Pipeline m_pipeline;
        vk::PipelineLayout m_pipelineLayout;
        std::vector<std::unique_ptr<Memory::Descriptor::SetLayout>> m_setLayouts;
        std::unordered_map<Core::Stage, const Core::Shader*> m_shaders;
    };
}
