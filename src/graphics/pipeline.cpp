//
// Created by radue on 10/17/2024.
//

#include "pipeline.h"

#include <iostream>
#include <ranges>

#include "../shader/shader.h"
#include "memory/descriptor/set.h"
#include "utils/functionals.h"

namespace Graphics {

    Pipeline::Builder::Builder() {
        m_inputAssembly = vk::PipelineInputAssemblyStateCreateInfo()
            .setTopology(vk::PrimitiveTopology::eTriangleList)
            .setPrimitiveRestartEnable(vk::False);

        m_rasterizer = vk::PipelineRasterizationStateCreateInfo()
            .setDepthClampEnable(vk::False)
            .setRasterizerDiscardEnable(vk::False)
            .setPolygonMode(vk::PolygonMode::eFill)
            .setCullMode(vk::CullModeFlagBits::eNone)
            .setFrontFace(vk::FrontFace::eCounterClockwise)
            .setDepthBiasEnable(vk::False)
            .setLineWidth(1.0f);

        m_depthStencil = vk::PipelineDepthStencilStateCreateInfo()
            .setDepthTestEnable(vk::True)
            .setDepthWriteEnable(vk::True)
            .setDepthCompareOp(vk::CompareOp::eLess)
            .setDepthBoundsTestEnable(vk::False)
            .setStencilTestEnable(vk::False);
    }

    Pipeline::Builder &Pipeline::Builder::AddShader(Core::Shader* shader) {
        const auto stage = shader->Stage();
        m_shaders[stage] = shader;
        return *this;
    }

    Pipeline::Builder &Pipeline::Builder::VertexInputState(const vk::PipelineVertexInputStateCreateInfo &vertexInputInfo)
    {
        m_vertexInputInfo = vertexInputInfo;
        return *this;
    }

    Pipeline::Builder &Pipeline::Builder::InputAssemblyState(const vk::PipelineInputAssemblyStateCreateInfo &inputAssembly)
    {
        m_inputAssembly = inputAssembly;
        return *this;
    }

    Pipeline::Builder &Pipeline::Builder::Viewport(const vk::Viewport &viewport)
    {
        m_viewports.push_back(viewport);
        return *this;
    }

    Pipeline::Builder &Pipeline::Builder::Scissor(const vk::Rect2D &scissor)
    {
        m_scissors.push_back(scissor);
        return *this;
    }

    Pipeline::Builder &Pipeline::Builder::Rasterizer(const vk::PipelineRasterizationStateCreateInfo &rasterizer)
    {
        m_rasterizer = rasterizer;
        return *this;
    }

    Pipeline::Builder &Pipeline::Builder::Multisampling(const vk::PipelineMultisampleStateCreateInfo &multisampling)
    {
        m_multisampling = multisampling;
        return *this;
    }

    Pipeline::Builder &Pipeline::Builder::DepthStencil(const vk::PipelineDepthStencilStateCreateInfo &depthStencil)
    {
        m_depthStencil = depthStencil;
        return *this;
    }

    Pipeline::Builder &Pipeline::Builder::ColorBlendAttachment(const vk::PipelineColorBlendAttachmentState &colorBlendAttachment)
    {
        m_colorBlendAttachments.push_back(colorBlendAttachment);
        return *this;
    }

    Pipeline::Builder &Pipeline::Builder::ColorBlend(const vk::PipelineColorBlendStateCreateInfo &colorBlending)
    {
        m_colorBlending = colorBlending;
        return *this;
    }

    Pipeline::Builder &Pipeline::Builder::DynamicState(const vk::DynamicState &dynamicState)
    {
        m_dynamicStates.push_back(dynamicState);
        return *this;
    }

    Pipeline::Builder &Pipeline::Builder::Tessellation(const vk::PipelineTessellationStateCreateInfo &tessellation)
    {
        m_tessellation = tessellation;
        return *this;
    }

    Pipeline::Builder &Pipeline::Builder::RenderPass(const vk::RenderPass &renderPass)
    {
        m_renderPass = renderPass;
        return *this;
    }

    Pipeline::Builder &Pipeline::Builder::Subpass(const uint32_t subpass)
    {
        m_subpass = subpass;
        return *this;
    }

    std::unique_ptr<Pipeline> Pipeline::Builder::Build()
    {
        std::vector<Memory::Descriptor::SetLayout::Builder> layoutBuilders;
        std::vector<vk::PushConstantRange> pushConstantRanges;
        for (const auto& shader : m_shaders | std::views::values) {
            for (const auto& layout : shader->Descriptors()) {
                if (layout.set >= layoutBuilders.size()) {
                    layoutBuilders.resize(layout.set + 1);
                }
                auto& currentLayout = layoutBuilders[layout.set];
                if (currentLayout.HasBinding(layout.binding)) {
                    currentLayout.Binding(layout.binding).stageFlags |= shader->Stage();
                } else {
                    currentLayout.AddBinding(layout.binding, layout.type, shader->Stage(), layout.count);
                }
            }
            for (const auto&[size, offset] : shader->PushConstantRanges()) {
                auto foundRange = Utils::FindIf(pushConstantRanges,
                [size, offset] (const auto& range) -> bool {
                    return range.offset == offset && range.size == size;
                });

                if (foundRange.has_value()) {
                    foundRange->stageFlags |= shader->Stage();
                } else {
                    pushConstantRanges.emplace_back(
                        vk::PushConstantRange()
                            .setOffset(offset)
                            .setSize(size)
                            .setStageFlags(shader->Stage()));
                }
            }
        }

        for (auto& layoutBuilder : layoutBuilders) {
            m_setLayouts.emplace_back(layoutBuilder.Build());
        }

        std::vector<vk::DescriptorSetLayout> descriptorHandles = m_setLayouts
            | std::views::transform([](const auto& layout) { return **layout; })
            | std::ranges::to<std::vector<vk::DescriptorSetLayout>>();

        const auto pipelineLayoutInfo = vk::PipelineLayoutCreateInfo()
            .setSetLayouts(descriptorHandles)
            .setPushConstantRanges(pushConstantRanges);

        m_pipelineLayout = Core::GlobalDevice()->createPipelineLayout(pipelineLayoutInfo);

        m_stages.clear();
        for (const auto &[stage, shader] : m_shaders) {
            m_stages.emplace_back(vk::PipelineShaderStageCreateInfo()
                .setStage(stage)
                .setModule(**shader)
                .setPName("main"));
        }

        if (!m_viewports.empty() && !m_scissors.empty()) {
            m_viewportState = vk::PipelineViewportStateCreateInfo()
                .setViewports(m_viewports)
                .setScissors(m_scissors);
        } else {
            m_viewportState = vk::PipelineViewportStateCreateInfo()
                .setViewportCount(1)
                .setScissorCount(1);
        }

        m_dynamicState = vk::PipelineDynamicStateCreateInfo()
            .setDynamicStates(m_dynamicStates);

        m_colorBlending
            .setLogicOpEnable(vk::False)
            .setLogicOp(vk::LogicOp::eCopy)
            .setAttachments(m_colorBlendAttachments);

        return std::make_unique<Pipeline>(*this);
    }

    Pipeline::Pipeline(Builder &builder)
        : m_pipelineLayout(builder.m_pipelineLayout), m_setLayouts(std::move(builder.m_setLayouts)), m_shaders(builder.m_shaders)
    {
        const auto m_createInfo = vk::GraphicsPipelineCreateInfo()
            .setStages(builder.m_stages)
            .setPVertexInputState(&builder.m_vertexInputInfo)
            .setPInputAssemblyState(&builder.m_inputAssembly)
            .setPViewportState(&builder.m_viewportState)
            .setPRasterizationState(&builder.m_rasterizer)
            .setPDepthStencilState(&builder.m_depthStencil)
            .setPColorBlendState(&builder.m_colorBlending)
            .setPMultisampleState(&builder.m_multisampling)
            .setPTessellationState(&builder.m_tessellation)
            .setPDynamicState(&builder.m_dynamicState)
            .setLayout(builder.m_pipelineLayout)
            .setRenderPass(builder.m_renderPass)
            .setSubpass(builder.m_subpass);

        const auto pipeline = Core::GlobalDevice()->createGraphicsPipeline(nullptr, m_createInfo);

        if (pipeline.result != vk::Result::eSuccess) {
            std::cerr << "Failed to create graphics pipeline: " << vk::to_string(pipeline.result) << std::endl;
        }
        m_pipeline = pipeline.value;
    }

    Pipeline::~Pipeline()
    {
        Core::GlobalDevice()->waitIdle();
        Core::GlobalDevice()->destroyPipeline(m_pipeline);
        Core::GlobalDevice()->destroyPipelineLayout(m_pipelineLayout);
    }

    void Pipeline::Bind(const vk::CommandBuffer& commandBuffer) const {
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline);
    }

    void Pipeline::BindDescriptorSet(const uint32_t setNumber, const vk::CommandBuffer commandBuffer, const Memory::Descriptor::Set &descriptorSet) const {
        commandBuffer.bindDescriptorSets(
            vk::PipelineBindPoint::eGraphics,
            m_pipelineLayout,
            setNumber,
            descriptorSet.Handle(),
            nullptr);
    }

    void Pipeline::BindDescriptorSets(const uint32_t startingSet, const vk::CommandBuffer commandBuffer, const std::vector<Memory::Descriptor::Set>& descriptorSets) const {
        std::vector<vk::DescriptorSet> sets;
        for (const auto &descriptorSet : descriptorSets) {
            sets.push_back(descriptorSet.Handle());
        }

        commandBuffer.bindDescriptorSets(
            vk::PipelineBindPoint::eGraphics,
            m_pipelineLayout,
            startingSet,
            sets,
            nullptr);
    }
}
