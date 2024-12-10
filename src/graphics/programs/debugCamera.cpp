//
// Created by radue on 12/8/2024.
//

#include "debugCamera.h"

#include "renderer.h"
#include "graphics/pipeline.h"
#include "graphics/renderPass.h"
#include "graphics/objects/mesh.h"
#include "renderPasses/graphicsPass.h"

DebugCamera::DebugCamera(const mgv::Camera &camera)
    : Program(
        {
            mgv::Renderer::GraphicsPass().RenderPass()
        }
    ), m_camera(camera)
{
    const auto renderPass = mgv::Renderer::GraphicsPass().RenderPass();

    const auto vertexAttributeDescriptions = mgv::Mesh::Vertex::GetAttributeDescriptions({mgv::Mesh::Attribute::Position});
    const auto vertexBindingDescriptions = mgv::Mesh::Vertex::GetBindingDescriptions();

    m_pipelines[renderPass] = Graphics::Pipeline::Builder()
        .AddShader("shaders/wireframe.vert")
        .AddShader("shaders/wireframe.frag")
        .DescriptorSetLayout(0, mgv::Renderer::GlobalSetLayout())
        .DescriptorSetLayout(1, *m_setLayout)
        .PushConstantRange<glm::uint>(vk::ShaderStageFlagBits::eVertex)
        .ColorBlendAttachment(vk::PipelineColorBlendAttachmentState()
            .setBlendEnable(vk::False)
            .setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA))
        .RenderPass(**renderPass)
        .Subpass(0)
        .Multisampling(vk::PipelineMultisampleStateCreateInfo()
            .setRasterizationSamples(renderPass->SampleCount()))
        .VertexInputState(vk::PipelineVertexInputStateCreateInfo()
            .setVertexAttributeDescriptions(vertexAttributeDescriptions)
            .setVertexBindingDescriptions(vertexBindingDescriptions))
        .Rasterizer(vk::PipelineRasterizationStateCreateInfo()
            .setPolygonMode(vk::PolygonMode::eLine)
            .setLineWidth(1.0f))
        .Build();
}

void DebugCamera::Init() {
}

void DebugCamera::Update(double deltaTime) {
}

void DebugCamera::Draw(const vk::CommandBuffer &commandBuffer, const Graphics::RenderPass* renderPass) const {

}

void DebugCamera::ResetDescriptorSets() {
}
