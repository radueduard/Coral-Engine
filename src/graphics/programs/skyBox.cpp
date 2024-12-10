//
// Created by radue on 11/29/2024.
//

#include "skyBox.h"

#include "lake.h"
#include "renderer.h"
#include "extensions/meshShader.h"
#include "graphics/pipeline.h"
#include "graphics/renderPass.h"
#include "graphics/objects/cubeMap.h"
#include "memory/descriptor/set.h"
#include "renderPasses/graphicsPass.h"
#include "renderPasses/reflectionPass.h"

SkyBox::SkyBox(const CreateInfo &createInfo)
    : Program({
        mgv::Renderer::ReflectionPass().RenderPass(),
        mgv::Renderer::GraphicsPass().RenderPass()
    }), m_cubeMap(createInfo.cubeMap)
{
    m_setLayout = Memory::Descriptor::SetLayout::Builder()
        .AddBinding(0, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment)
        .Build();

    const auto reflectionPass = mgv::Renderer::ReflectionPass().RenderPass();
    const auto graphicsPass = mgv::Renderer::GraphicsPass().RenderPass();

    auto pipelineBuilder = Graphics::Pipeline::Builder();

    pipelineBuilder
        .AddShader("shaders/skyBox/skyBox.mesh")
        .AddShader("shaders/skyBox/skyBox.frag")
        .ColorBlendAttachment(vk::PipelineColorBlendAttachmentState()
            .setBlendEnable(vk::False)
            .setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA))
        .RenderPass(**reflectionPass)
        .DescriptorSetLayout(0, mgv::Renderer::GlobalSetLayout())
        .DescriptorSetLayout(1, *m_setLayout)
        .PushConstantRange<glm::uint>(vk::ShaderStageFlagBits::eMeshEXT)
        .Subpass(0)
        .Multisampling(vk::PipelineMultisampleStateCreateInfo()
            .setRasterizationSamples(reflectionPass->SampleCount()))
        .DepthStencil(vk::PipelineDepthStencilStateCreateInfo()
            .setDepthTestEnable(vk::False));

    m_pipelines[reflectionPass] = pipelineBuilder.Build();

    pipelineBuilder
        .RenderPass(**graphicsPass)
        .Multisampling(vk::PipelineMultisampleStateCreateInfo()
            .setRasterizationSamples(graphicsPass->SampleCount()));

    m_pipelines[graphicsPass] = pipelineBuilder.Build();

    ResetDescriptorSets();
}

void SkyBox::Draw(const vk::CommandBuffer &commandBuffer, const Graphics::RenderPass* renderPass) const {
    const auto reflected = renderPass == mgv::Renderer::ReflectionPass().RenderPass();
    const auto& pipeline = m_pipelines.at(renderPass);

    pipeline->Bind(commandBuffer);
    pipeline->BindDescriptorSet(0, commandBuffer, mgv::Renderer::GlobalDescriptorSet());
    pipeline->BindDescriptorSet(1, commandBuffer, *m_descriptorSet);
    pipeline->PushConstants(commandBuffer, vk::ShaderStageFlagBits::eMeshEXT, 0, reflected ? 1u : 0u);
    Ext::MeshShader::cmdDrawMeshTasks(commandBuffer, 1, 1, 1);
}

void SkyBox::ResetDescriptorSets() {
    m_descriptorSet = Memory::Descriptor::Set::Builder(mgv::Renderer::DescriptorPool(), *m_setLayout)
        .WriteImage(0, m_cubeMap.DescriptorInfo())
        .Build();

    OnUIReset();
}
