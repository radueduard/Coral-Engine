//
// Created by radue on 12/11/2024.
//

#include "trajectoryDisplay.h"

#include "renderer.h"
#include "extensions/meshShader.h"
#include "graphics/renderPass.h"
#include "memory/descriptor/set.h"
#include "renderPasses/graphicsPass.h"

#include <glm/glm.hpp>

TrajectoryDisplay::TrajectoryDisplay(const CreateInfo &createInfo)
    : Program({
        mgv::Renderer::GraphicsPass().RenderPass()
    }), m_particlesBuffer(createInfo.particlesBuffer), m_trajectoriesBuffer(createInfo.trajectoriesBuffer)
{
    m_setLayout = Memory::Descriptor::SetLayout::Builder()
        .AddBinding(0, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eMeshEXT)
        .AddBinding(1, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eMeshEXT)
        .Build();

    const auto graphicsPass = mgv::Renderer::GraphicsPass().RenderPass();

    m_pipelines[graphicsPass] = Graphics::Pipeline::Builder()
        .AddShader("shaders/trajectory/trajectory.mesh")
        .AddShader("shaders/trajectory/trajectory.frag")
        .DescriptorSetLayout(0, mgv::Renderer::GlobalSetLayout())
        .DescriptorSetLayout(1, *m_setLayout)
        .ColorBlendAttachment(vk::PipelineColorBlendAttachmentState()
            .setBlendEnable(vk::False)
            .setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA))
        .RenderPass(**graphicsPass)
        .Subpass(0)
        .Multisampling(vk::PipelineMultisampleStateCreateInfo()
            .setRasterizationSamples(graphicsPass->SampleCount()))
        .Build();

        ResetDescriptorSets();
}

void TrajectoryDisplay::Draw(const vk::CommandBuffer &commandBuffer, const Graphics::RenderPass *renderPass) const {
    const auto& pipeline = m_pipelines.at(renderPass);
    pipeline->Bind(commandBuffer);
    pipeline->BindDescriptorSet(0, commandBuffer, mgv::Renderer::GlobalDescriptorSet());
    pipeline->BindDescriptorSet(1, commandBuffer, *m_descriptorSet);

    const uint32_t instanceCount = m_particlesBuffer.InstanceCount();
    Ext::MeshShader::cmdDrawMeshTasks(commandBuffer, static_cast<uint32_t>(sqrt(instanceCount)), static_cast<uint32_t>(sqrt(instanceCount)), 1);
}

void TrajectoryDisplay::ResetDescriptorSets() {
    m_descriptorSet = Memory::Descriptor::Set::Builder(mgv::Renderer::DescriptorPool(), *m_setLayout)
        .WriteBuffer(0, m_particlesBuffer.DescriptorInfo())
        .WriteBuffer(1, m_trajectoriesBuffer.DescriptorInfo())
        .Build();
}