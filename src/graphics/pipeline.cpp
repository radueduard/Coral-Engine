//
// Created by radue on 10/17/2024.
//

#include "pipeline.h"

#include <iostream>
#include <ranges>

namespace Graphics {

    Pipeline::Builder::Builder() {
        m_yetPossibleTypes = std::unordered_set {VTG, TM, RT};

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

    Pipeline::Builder &Pipeline::Builder::AddShader(const Core::Shader &shader)
    {
        const auto stage = shader.Stage();
        const auto module = *shader;

        const auto vtgStages = std::unordered_set {
            vk::ShaderStageFlagBits::eVertex,
            vk::ShaderStageFlagBits::eTessellationControl,
            vk::ShaderStageFlagBits::eTessellationEvaluation,
            vk::ShaderStageFlagBits::eGeometry,
            vk::ShaderStageFlagBits::eFragment,
        };

        const auto tmStages = std::unordered_set {
            vk::ShaderStageFlagBits::eTaskEXT,
            vk::ShaderStageFlagBits::eMeshEXT,
            vk::ShaderStageFlagBits::eFragment,
        };

        const auto rtStages = std::unordered_set {
            vk::ShaderStageFlagBits::eRaygenKHR,
            vk::ShaderStageFlagBits::eAnyHitKHR,
            vk::ShaderStageFlagBits::eClosestHitKHR,
            vk::ShaderStageFlagBits::eMissKHR,
            vk::ShaderStageFlagBits::eIntersectionKHR,
            vk::ShaderStageFlagBits::eCallableKHR,
        };

        if ((!m_yetPossibleTypes.contains(VTG) && vtgStages.contains(stage)) &&
            (!m_yetPossibleTypes.contains(TM) && tmStages.contains(stage)) &&
            (!m_yetPossibleTypes.contains(RT) && rtStages.contains(stage)))
        {
            const std::string errorMessage =
            "You can't have shaders of different types in the same pipeline\n"
            "You can add shaders within the following types:\n"
                "\t- VTG: Vertex, Tessellation Control, Tessellation Evaluation, Geometry, Fragment\n"
                "\t- TM: Task, Mesh, Fragment\n"
                "\t- RT: Ray Generation, Any Hit, Closest Hit, Miss, Intersection, Callable\n";
            throw std::runtime_error(errorMessage);
        }

        if (stage == vk::ShaderStageFlagBits::eFragment) {
            m_yetPossibleTypes.erase(RT);
        }
        else if (vtgStages.contains(stage)) {
            m_yetPossibleTypes.erase(TM);
            m_yetPossibleTypes.erase(RT);
        } else if (tmStages.contains(stage)) {
            m_yetPossibleTypes.erase(VTG);
            m_yetPossibleTypes.erase(RT);
        } else if (rtStages.contains(stage)) {
            m_yetPossibleTypes.erase(VTG);
            m_yetPossibleTypes.erase(TM);
        }

        m_shaders[stage] = module;
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

    Pipeline::Builder &Pipeline::Builder::PipelineLayout(const vk::PipelineLayout &pipelineLayout)
    {
        m_pipelineLayout = pipelineLayout;
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

    Pipeline::Builder &Pipeline::Builder::BasePipeline(const vk::Pipeline &basePipeline, const int32_t basePipelineIndex)
    {
        m_basePipeline = basePipeline;
        m_basePipelineIndex = basePipelineIndex;
        return *this;
    }

    void Pipeline::Builder::CheckShaderStagesValidity() const {
        if (m_yetPossibleTypes.size() != 1) {
            throw std::runtime_error("Not enough shaders added to the pipeline");
        }
        switch (*m_yetPossibleTypes.begin()) {
            case VTG:
                if (!m_shaders.contains(vk::ShaderStageFlagBits::eVertex) ||
                    !m_shaders.contains(vk::ShaderStageFlagBits::eFragment)) {
                    throw std::runtime_error("VTG pipeline: At least Vertex and Fragment shaders are required");
                }
            break;
            case TM:
                if (!m_shaders.contains(vk::ShaderStageFlagBits::eMeshEXT) ||
                    !m_shaders.contains(vk::ShaderStageFlagBits::eFragment)) {
                    throw std::runtime_error("TM pipeline: At least Mesh and Fragment shaders are required");
                }
            break;
            case RT:
                // TODO: Check validity of error message
                if (!m_shaders.contains(vk::ShaderStageFlagBits::eRaygenKHR) ||
                    !m_shaders.contains(vk::ShaderStageFlagBits::eMissKHR)) {
                    throw std::runtime_error("RT pipeline: At least Ray Generation and Miss shaders are required");
                }
            break;
        }
    }

    std::unique_ptr<Pipeline> Pipeline::Builder::Build(const Core::Device &device)
    {
        CheckShaderStagesValidity();

        for (const auto &[stage, shader] : m_shaders) {
            m_stages.push_back(vk::PipelineShaderStageCreateInfo()
                .setStage(stage)
                .setModule(shader)
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

        return std::make_unique<Pipeline>(device, *this);
    }

    Pipeline::Pipeline(const Core::Device &device, const Builder &builder)
        : m_device(device)
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
            .setSubpass(builder.m_subpass)
            .setBasePipelineHandle(builder.m_basePipeline)
            .setBasePipelineIndex(builder.m_basePipelineIndex);

        const auto pipeline = (*device).createGraphicsPipeline(nullptr, m_createInfo);
        if (pipeline.result != vk::Result::eSuccess) {
            std::cerr << "Failed to create graphics pipeline: " << vk::to_string(pipeline.result) << std::endl;
        }
        m_pipeline = pipeline.value;
        m_type = *builder.m_yetPossibleTypes.begin();
        m_shaders = std::move(builder.m_shaders);
    }

    Pipeline::~Pipeline()
    {
        (*m_device).destroyPipeline(m_pipeline);
    }

    void Pipeline::Bind(const vk::CommandBuffer& commandBuffer) const {
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline);
    }
}
