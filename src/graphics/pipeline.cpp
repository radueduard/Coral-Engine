//
// Created by radue on 10/17/2024.
//

#include "pipeline.h"

#include <iostream>
#include <ranges>

#include "shader/shader.h"
#include "memory/descriptor/set.h"
#include "objects/mesh.h"
#include "renderPass.h"

namespace Coral::Graphics {
    Pipeline::BuilderRenderPass::BuilderRenderPass(RenderPass &renderPass) : m_renderPass(renderPass) {
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

    Pipeline::BuilderRenderPass &Pipeline::BuilderRenderPass::AddShader(const Shader::Shader* shader) {
        const auto stage = shader->GetStage();
        m_shaders[stage] = shader;
        return *this;
    }

	Pipeline::BuilderDynamic &Pipeline::BuilderDynamic::AddShader(const Shader::Shader* shader) {
    	const auto stage = shader->GetStage();
    	m_shaders[stage] = shader;
    	return *this;
    }

    Pipeline::BuilderRenderPass &Pipeline::BuilderRenderPass::InputAssemblyState(const vk::PipelineInputAssemblyStateCreateInfo &inputAssembly)
    {
        m_inputAssembly = inputAssembly;
        return *this;
    }

    Pipeline::BuilderRenderPass &Pipeline::BuilderRenderPass::Viewport(const vk::Viewport &viewport)
    {
        m_viewports.push_back(viewport);
        return *this;
    }

    Pipeline::BuilderRenderPass &Pipeline::BuilderRenderPass::Scissor(const vk::Rect2D &scissor)
    {
        m_scissors.push_back(scissor);
        return *this;
    }

    Pipeline::BuilderRenderPass &Pipeline::BuilderRenderPass::Rasterizer(const vk::PipelineRasterizationStateCreateInfo &rasterizer)
    {
        m_rasterizer = rasterizer;
        return *this;
    }

    Pipeline::BuilderRenderPass &Pipeline::BuilderRenderPass::DepthStencil(const vk::PipelineDepthStencilStateCreateInfo &depthStencil)
    {
        m_depthStencil = depthStencil;
        return *this;
    }

    Pipeline::BuilderRenderPass &Pipeline::BuilderRenderPass::DynamicState(const vk::DynamicState &dynamicState)
    {
        m_dynamicStates.push_back(dynamicState);
        return *this;
    }

    Pipeline::BuilderRenderPass &Pipeline::BuilderRenderPass::Tessellation(const vk::PipelineTessellationStateCreateInfo &tessellation)
    {
        m_tessellation = tessellation;
        return *this;
    }

    Pipeline::BuilderRenderPass &Pipeline::BuilderRenderPass::Subpass(const uint32_t subpass)
    {
        m_subpass = subpass;
        return *this;
    }

	Pipeline::BuilderRenderPass& Pipeline::BuilderRenderPass::RenderFunction(const std::function<void(const Graphics::Pipeline&, const Core::CommandBuffer&)>& function)
	{
	    m_function = function;
		return *this;
    }

    std::unique_ptr<Pipeline> Pipeline::BuilderRenderPass::Build()
    {
        std::vector<Memory::Descriptor::SetLayout::Builder> layoutBuilders;
        std::vector<vk::PushConstantRange> pushConstantRanges;
        for (const auto& shader : m_shaders | std::views::values) {
            for (const auto& descriptor : shader->Descriptors()) {
                if (descriptor.set >= layoutBuilders.size()) {
                    layoutBuilders.resize(descriptor.set + 1);
                }
                if (auto& currentLayout = layoutBuilders[descriptor.set]; currentLayout.HasBinding(descriptor.binding)) {
                    currentLayout.Binding(descriptor.binding).stageFlags |= vk::ShaderStageFlags(static_cast<uint32_t>(shader->GetStage()));
                } else {
                    currentLayout.AddBinding(descriptor.binding, descriptor.type, vk::ShaderStageFlags(static_cast<uint32_t>(shader->GetStage())), descriptor.count);
                }
            }
            for (const auto&[size, offset, name] : shader->PushConstantRanges()) {
                std::optional<vk::PushConstantRange> foundRange = std::nullopt;
            	for (const auto& range : pushConstantRanges) {
            		if (range.offset == offset && range.size == size) {
						foundRange = range;
						break;
					}
            	}

                if (foundRange.has_value()) {
                    foundRange->stageFlags |= vk::ShaderStageFlags(static_cast<uint32_t>(shader->GetStage()));
                } else {
                    pushConstantRanges.emplace_back(
                        vk::PushConstantRange()
                            .setOffset(offset)
                            .setSize(size)
                            .setStageFlags(vk::ShaderStageFlags(static_cast<uint32_t>(shader->GetStage()))));
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

        m_pipelineLayout = Context::Device()->createPipelineLayout(pipelineLayoutInfo);

        std::vector<vk::VertexInputBindingDescription> bindingDescriptions = {};
        std::vector<vk::VertexInputAttributeDescription> attributeDescriptions = {};

    	std::optional<const Shader::Shader*> vertexShader = std::nullopt;
    	for (auto* shader : m_shaders | std::views::values) {
    		if (shader->GetStage() == Shader::Stage::Vertex) {
    			vertexShader = shader;
    			break;
    		}
    	}
        if (vertexShader.has_value()) {
            bindingDescriptions = Vertex::BindingDescriptions();
            attributeDescriptions = Vertex::AttributeDescriptions((*vertexShader)->Inputs());
        }

        m_vertexInputInfo = vk::PipelineVertexInputStateCreateInfo()
            .setVertexBindingDescriptions(bindingDescriptions)
            .setVertexAttributeDescriptions(attributeDescriptions);

        m_stages.clear();
        for (const auto& shader : m_shaders | std::views::values) {
            m_stages.emplace_back(vk::PipelineShaderStageCreateInfo()
                .setStage(static_cast<vk::ShaderStageFlagBits>(shader->GetStage()))
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

        m_colorBlendAttachments.clear();
    	auto subpass = m_renderPass.SubpassColorAttachments(m_subpass);
        for (const auto& attachment : subpass) {
            m_colorBlendAttachments.emplace_back(vk::PipelineColorBlendAttachmentState()
                .setBlendEnable(vk::False)
                .setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA));
        }

        m_colorBlending
            .setLogicOpEnable(vk::False)
            .setLogicOp(vk::LogicOp::eCopy)
            .setAttachments(m_colorBlendAttachments);

        m_multisampling = vk::PipelineMultisampleStateCreateInfo()
            .setRasterizationSamples(m_renderPass.SampleCount())
            .setSampleShadingEnable(vk::False)
            .setMinSampleShading(1.0f)
            .setAlphaToCoverageEnable(vk::False)
            .setAlphaToOneEnable(vk::False);

        return std::make_unique<Pipeline>(*this);
    }

	Pipeline::BuilderDynamic& Pipeline::BuilderDynamic::InputAssemblyState(const vk::PipelineInputAssemblyStateCreateInfo& inputAssembly) {
	    m_inputAssembly = inputAssembly;
	    return *this;
    }
	Pipeline::BuilderDynamic& Pipeline::BuilderDynamic::Rasterizer(const vk::PipelineRasterizationStateCreateInfo& rasterizer) {
	    m_rasterizer = rasterizer;
	    return *this;
    }
	Pipeline::BuilderDynamic& Pipeline::BuilderDynamic::DepthStencil(const vk::PipelineDepthStencilStateCreateInfo& depthStencil) {
	    m_depthStencil = depthStencil;
	    return *this;
    }
	Pipeline::BuilderDynamic& Pipeline::BuilderDynamic::DynamicState(const vk::DynamicState& dynamicState) {
	    m_dynamicStates.push_back(dynamicState);
	    return *this;
    }
	Pipeline::BuilderDynamic& Pipeline::BuilderDynamic::Tessellation(const vk::PipelineTessellationStateCreateInfo& tesselationInfo) {
	    m_tessellation = tesselationInfo;
	    return *this;
    }
	Pipeline::BuilderDynamic& Pipeline::BuilderDynamic::Multisampling(const vk::PipelineMultisampleStateCreateInfo& multisampling) {
    	m_multisampling = multisampling;
	    return *this;
    }
	Pipeline::BuilderDynamic& Pipeline::BuilderDynamic::RenderFunction(const std::function<void(const Pipeline&, const Core::CommandBuffer&)>& function) {
	    m_function = function;
	    return *this;
    }
	std::unique_ptr<Pipeline> Pipeline::BuilderDynamic::Build() {
	    std::vector<Memory::Descriptor::SetLayout::Builder> layoutBuilders;
        std::vector<vk::PushConstantRange> pushConstantRanges;
        for (const auto& shader : m_shaders | std::views::values) {
            for (const auto& descriptor : shader->Descriptors()) {
                if (descriptor.set >= layoutBuilders.size()) {
                    layoutBuilders.resize(descriptor.set + 1);
                }
                if (auto& currentLayout = layoutBuilders[descriptor.set]; currentLayout.HasBinding(descriptor.binding)) {
                    currentLayout.Binding(descriptor.binding).stageFlags |= vk::ShaderStageFlags(static_cast<uint32_t>(shader->GetStage()));
                } else {
                    currentLayout.AddBinding(descriptor.binding, descriptor.type, vk::ShaderStageFlags(static_cast<uint32_t>(shader->GetStage())), descriptor.count);
                }
            }
            for (const auto&[size, offset, name] : shader->PushConstantRanges()) {
                std::optional<vk::PushConstantRange> foundRange = std::nullopt;
            	for (const auto& range : pushConstantRanges) {
            		if (range.offset == offset && range.size == size) {
						foundRange = range;
						break;
					}
            	}

                if (foundRange.has_value()) {
                    foundRange->stageFlags |= vk::ShaderStageFlags(static_cast<uint32_t>(shader->GetStage()));
                } else {
                    pushConstantRanges.emplace_back(
                        vk::PushConstantRange()
                            .setOffset(offset)
                            .setSize(size)
                            .setStageFlags(vk::ShaderStageFlags(static_cast<uint32_t>(shader->GetStage()))));
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

        m_pipelineLayout = Context::Device()->createPipelineLayout(pipelineLayoutInfo);

        std::vector<vk::VertexInputBindingDescription> bindingDescriptions = {};
        std::vector<vk::VertexInputAttributeDescription> attributeDescriptions = {};

    	std::optional<const Shader::Shader*> vertexShader = std::nullopt;
    	for (auto* shader : m_shaders | std::views::values) {
    		if (shader->GetStage() == Shader::Stage::Vertex) {
    			vertexShader = shader;
    			break;
    		}
    	}
        if (vertexShader.has_value()) {
            bindingDescriptions = Vertex::BindingDescriptions();
            attributeDescriptions = Vertex::AttributeDescriptions((*vertexShader)->Inputs());
        }

        m_vertexInputInfo = vk::PipelineVertexInputStateCreateInfo()
            .setVertexBindingDescriptions(bindingDescriptions)
            .setVertexAttributeDescriptions(attributeDescriptions);

        m_stages.clear();
        for (const auto& shader : m_shaders | std::views::values) {
            m_stages.emplace_back(vk::PipelineShaderStageCreateInfo()
                .setStage(static_cast<vk::ShaderStageFlagBits>(shader->GetStage()))
                .setModule(**shader)
                .setPName("main"));
        }

        m_dynamicState = vk::PipelineDynamicStateCreateInfo()
            .setDynamicStates(m_dynamicStates);

    	m_viewportState = vk::PipelineViewportStateCreateInfo()
			.setViewportCount(1)
			.setScissorCount(1);

        m_colorBlendAttachments.clear();
    	// auto subpass = m_renderPass.SubpassColorAttachments(m_subpass);
     //    for (const auto& attachment : subpass) {
     //        m_colorBlendAttachments.emplace_back(vk::PipelineColorBlendAttachmentState()
     //            .setBlendEnable(vk::False)
     //            .setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA));
     //    }
     //
     //    m_colorBlending
     //        .setLogicOpEnable(vk::False)
     //        .setLogicOp(vk::LogicOp::eCopy)
     //        .setAttachments(m_colorBlendAttachments);

        return std::make_unique<Pipeline>(*this);
    }

	Pipeline::Pipeline(BuilderRenderPass& builder)
		: m_pipelineLayout(builder.m_pipelineLayout),
		m_setLayouts(std::move(builder.m_setLayouts)),
		m_shaders(std::move(builder.m_shaders)),
		m_renderFunction(builder.m_function)
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
            .setRenderPass(*builder.m_renderPass)
            .setSubpass(builder.m_subpass);

        try {
            const auto pipeline = Context::Device()->createGraphicsPipeline(nullptr, m_createInfo);
            if (pipeline.result != vk::Result::eSuccess) {
                std::cerr << "Failed to create graphics pipeline: " << vk::to_string(pipeline.result) << std::endl;
            }
            m_pipeline = pipeline.value;
        } catch (const std::exception &e) {
            std::cerr << "Failed to create graphics pipeline: " << e.what() << std::endl;
        }
    }

	Pipeline::Pipeline(BuilderDynamic& builder)
		: m_pipelineLayout(builder.m_pipelineLayout),
		m_setLayouts(std::move(builder.m_setLayouts)),
		m_shaders(std::move(builder.m_shaders)),
		m_renderFunction(builder.m_function)
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
    		.setPNext(&builder.m_pipelineRenderingInfo);

    	try {
    		const auto pipeline = Context::Device()->createGraphicsPipeline(nullptr, m_createInfo);
    		if (pipeline.result != vk::Result::eSuccess) {
    			std::cerr << "Failed to create graphics pipeline: " << vk::to_string(pipeline.result) << std::endl;
    		}
    		m_pipeline = pipeline.value;
    	} catch (const std::exception &e) {
    		std::cerr << "Failed to create graphics pipeline: " << e.what() << std::endl;
    	}
    }

	Pipeline::~Pipeline()
    {
        Context::Device()->waitIdle();
        Context::Device()->destroyPipeline(m_pipeline);
        Context::Device()->destroyPipelineLayout(m_pipelineLayout);
    }

    void Pipeline::Bind(const vk::CommandBuffer& commandBuffer) const {
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline);
    }

    void Pipeline::BindDescriptorSet(const uint32_t setNumber, const vk::CommandBuffer commandBuffer, const Memory::Descriptor::Set &descriptorSet) const {
        commandBuffer.bindDescriptorSets(
            vk::PipelineBindPoint::eGraphics,
            m_pipelineLayout,
            setNumber,
            *descriptorSet,
            nullptr);
    }

    void Pipeline::BindDescriptorSets(const uint32_t startingSet, const vk::CommandBuffer commandBuffer, const std::vector<Memory::Descriptor::Set>& descriptorSets) const {
        std::vector<vk::DescriptorSet> sets;
        for (const auto &descriptorSet : descriptorSets) {
            sets.push_back(*descriptorSet);
        }

        commandBuffer.bindDescriptorSets(
            vk::PipelineBindPoint::eGraphics,
            m_pipelineLayout,
            startingSet,
            sets,
            nullptr);
    }
}
