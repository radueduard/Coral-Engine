//
// Created by radue on 12/1/2024.
//

#include "wireFrameProgram.h"

#include "components/renderMesh.h"
#include "graphics/renderPass.h"
#include "graphics/objects/mesh.h"

WireFrameProgram::WireFrameProgram(Graphics::RenderPass &renderPass, const Memory::Descriptor::Pool &pool) : Program(renderPass) {
    m_setLayout = Memory::Descriptor::SetLayout::Builder()
        .Build();

    const auto vertexAttributeDescriptions = mgv::Mesh::Vertex::GetAttributeDescriptions({mgv::Mesh::Attribute::Position});
    const auto vertexBindingDescriptions = mgv::Mesh::Vertex::GetBindingDescriptions();

    m_pipeline = Graphics::Pipeline::Builder()
        .AddShader(Core::Shader("shaders/debug/wireFrame.vert"))
        .AddShader(Core::Shader("shaders/debug/wireFrame.frag"))
        .ColorBlendAttachment(vk::PipelineColorBlendAttachmentState()
            .setBlendEnable(vk::False)
            .setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA))
        .RenderPass(*renderPass)
        .DescriptorSetLayout(0, *m_setLayout)
        .Subpass(0)
        .Multisampling(vk::PipelineMultisampleStateCreateInfo()
            .setRasterizationSamples(renderPass.SampleCount()))
        .VertexInputState(vk::PipelineVertexInputStateCreateInfo()
            .setVertexAttributeDescriptions(vertexAttributeDescriptions)
            .setVertexBindingDescriptions(vertexBindingDescriptions))
        .Rasterizer(vk::PipelineRasterizationStateCreateInfo()
            .setPolygonMode(vk::PolygonMode::eLine)
            .setLineWidth(1.0f))
        .Build();
}

void WireFrameProgram::Draw(const vk::CommandBuffer &commandBuffer, bool reflected) {
    if (reflected)
        return;

    m_pipeline->Bind(commandBuffer);
    m_pipeline->BindDescriptorSet(0, commandBuffer, *m_descriptorSet);

    for (const auto &object : m_objects) {
        auto renderTarget = object->Get<mgv::RenderMesh>();
        if (!renderTarget)
            continue;
        for (const auto &mesh: renderTarget.value()->Targets() | std::views::keys) {
            mesh->Bind(commandBuffer);
            mesh->Draw(commandBuffer, 1);
        }
    }
}
