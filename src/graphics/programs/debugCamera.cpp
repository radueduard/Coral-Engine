//
// Created by radue on 12/8/2024.
//

#include "debugCamera.h"

#include "renderer.h"
#include "components/camera.h"
#include "core/input.h"
#include "graphics/pipeline.h"
#include "graphics/renderPass.h"
#include "graphics/objects/mesh.h"
#include "renderPasses/graphicsPass.h"

DebugCamera::DebugCamera()
    : Program(
        {
            mgv::Renderer::GraphicsPass().RenderPass()
        }
    )
{
    const auto renderPass = mgv::Renderer::GraphicsPass().RenderPass();

    const auto vertexAttributeDescriptions = mgv::Mesh::Vertex::GetAttributeDescriptions({mgv::Mesh::Attribute::Position});
    const auto vertexBindingDescriptions = mgv::Mesh::Vertex::GetBindingDescriptions();

    m_pipelines[renderPass] = Graphics::Pipeline::Builder()
        .AddShader("shaders/wireframe/wireframe.vert")
        .AddShader("shaders/wireframe/wireframe.frag")
        .DescriptorSetLayout(0, mgv::Renderer::GlobalSetLayout())
        .PushConstantRange<glm::mat4>(vk::ShaderStageFlagBits::eVertex)
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
        .DynamicState(vk::DynamicState::eLineWidth)
        .Rasterizer(vk::PipelineRasterizationStateCreateInfo()
            .setPolygonMode(vk::PolygonMode::eLine)
            .setLineWidth(2.0f))
        .Build();
}

void DebugCamera::Init() {
    for (const auto* camera : mgv::Camera::All()) {
        m_meshes[camera] = mgv::Mesh::Frustum(camera);
    }
}

void DebugCamera::Update(const double deltaTime) {
    for (auto* camera : mgv::Camera::All()) {
        if (!m_meshes.contains(camera)) {
            m_meshes[camera] = mgv::Mesh::Frustum(camera);
        }
        camera->Update(deltaTime);

        if(camera->m_changed) {
            m_meshes[camera] = mgv::Mesh::Frustum(camera);
        }
    }
}

void DebugCamera::Draw(const vk::CommandBuffer &commandBuffer, const Graphics::RenderPass* renderPass) const {
    m_pipelines.at(renderPass)->Bind(commandBuffer);
    m_pipelines.at(renderPass)->BindDescriptorSet(0, commandBuffer, mgv::Renderer::GlobalDescriptorSet());

    for (const auto& [camera, mesh] : m_meshes) {
        if (camera->Primary())
            continue;

        auto model = glm::mat4(1.0f);
        auto forward = glm::quat(glm::vec3{0, glm::pi<float>(), 0} + glm::radians(camera->Owner().rotation));
        model = glm::translate(model, camera->Owner().position);
        model *= glm::mat4_cast(forward);

        m_pipelines.at(renderPass)->PushConstants(commandBuffer, vk::ShaderStageFlagBits::eVertex, 0, model);

        mesh->Bind(commandBuffer);
        mesh->Draw(commandBuffer, 1);
    }
}

void DebugCamera::ResetDescriptorSets() {}
