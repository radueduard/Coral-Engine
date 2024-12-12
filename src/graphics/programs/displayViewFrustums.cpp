//
// Created by radue on 12/11/2024.
//

#include "displayViewFrustums.h"

#include "imgui.h"
#include "renderer.h"
#include "extensions/meshShader.h"
#include "memory/descriptor/set.h"
#include "renderPasses/graphicsPass.h"

#include <glm/glm.hpp>

#include "components/camera.h"
#include "graphics/renderPass.h"

DisplayViewFrustums::DisplayViewFrustums(const CreateInfo &createInfo)
    : Program({
        mgv::Renderer::GraphicsPass().RenderPass()
    }), m_frustumsBuffer(createInfo.frustumsBuffer)
{
    m_setLayout = Memory::Descriptor::SetLayout::Builder()
        .AddBinding(0, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eMeshEXT)
        .Build();

    const auto renderPass = mgv::Renderer::GraphicsPass().RenderPass();

    m_pipelines[renderPass] = Graphics::Pipeline::Builder()
        .AddShader("shaders/wireframe/frustum.mesh")
        .AddShader("shaders/wireframe/wireframe.frag")
        .DescriptorSetLayout(0, mgv::Renderer::GlobalSetLayout())
        .DescriptorSetLayout(1, *m_setLayout)
        .PushConstantRange<glm::mat4>(vk::ShaderStageFlagBits::eMeshEXT)
        .ColorBlendAttachment(vk::PipelineColorBlendAttachmentState()
            .setBlendEnable(vk::False)
            .setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA))
        .RenderPass(**renderPass)
        .Subpass(0)
        .Multisampling(vk::PipelineMultisampleStateCreateInfo()
            .setRasterizationSamples(renderPass->SampleCount()))
        .Build();

    ResetDescriptorSets();
}

void DisplayViewFrustums::Draw(const vk::CommandBuffer &commandBuffer, const Graphics::RenderPass *renderPass) const {
    if (!m_enabled) {
        return;
    }
    const auto& pipeline = m_pipelines.at(renderPass);

    pipeline->Bind(commandBuffer);
    pipeline->BindDescriptorSet(0, commandBuffer, mgv::Renderer::GlobalDescriptorSet());
    pipeline->BindDescriptorSet(1, commandBuffer, *m_descriptorSet);
    pipeline->PushConstants(commandBuffer, vk::ShaderStageFlagBits::eMeshEXT, 0, mgv::Camera::Main()->Owner().Matrix());
    const auto instanceCount = m_frustumsBuffer.InstanceCount();
    Ext::MeshShader::cmdDrawMeshTasks(commandBuffer, static_cast<uint32_t>(sqrt(instanceCount)), static_cast<uint32_t>(sqrt(instanceCount)), 1);
}

void DisplayViewFrustums::ResetDescriptorSets() {
    m_descriptorSet = Memory::Descriptor::Set::Builder(mgv::Renderer::DescriptorPool(), *m_setLayout)
        .WriteBuffer(0, m_frustumsBuffer.DescriptorInfo())
        .Build();
}


void DisplayViewFrustums::OnUIRender() {
    ImGui::Begin("View Frustums");
    ImGui::Checkbox("Enabled", &m_enabled);
    ImGui::End();
}
