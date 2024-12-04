//
// Created by radue on 12/1/2024.
//

#include "firefliesDisplay.h"

#include "assets/importer.h"
#include "assets/manager.h"
#include "components/camera.h"
#include "graphics/renderPass.h"

struct Info {
    glm::mat4 viewProjection;
};

FirefliesDisplay::FirefliesDisplay(Graphics::RenderPass& renderPass, const Memory::Descriptor::Pool &pool, const CreateInfo &createInfo)
    : Program(renderPass), m_camera(createInfo.camera), m_particlesBuffer(createInfo.particlesBuffer), m_mesh(*mgv::Mesh::Sphere()) {

    const auto firefliesCreateInfo = Fireflies::CreateInfo {
        .heightMap = createInfo.heightMap,
        .particlesBuffer = createInfo.particlesBuffer
    };

    m_firefliesProgram = std::make_unique<Fireflies>(pool, firefliesCreateInfo);

    const auto partitionLightsCreateInfo = PartitionLights::CreateInfo {
        .chunksPerAxis = 30,
        .particlesBuffer = createInfo.particlesBuffer
    };

    m_partitionLightsProgram = std::make_unique<PartitionLights>(pool, partitionLightsCreateInfo);

    m_setLayout = Memory::Descriptor::SetLayout::Builder()
        .AddBinding(0, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eVertex)
        .Build();

    const auto vertexAttributeDescriptions = mgv::Mesh::Vertex::GetAttributeDescriptions({mgv::Mesh::Attribute::Position});
    const auto vertexBindingDescriptions = mgv::Mesh::Vertex::GetBindingDescriptions();

    m_pipeline = Graphics::Pipeline::Builder()
        .AddShader(Core::Shader("shaders/fireflies/fireflies.vert"))
        .AddShader(Core::Shader("shaders/fireflies/fireflies.frag"))
        .DescriptorSetLayout(0, *m_setLayout)
        .PushConstantRange<Info>(vk::ShaderStageFlagBits::eVertex)
        .ColorBlendAttachment(vk::PipelineColorBlendAttachmentState()
            .setBlendEnable(vk::False)
            .setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA))
        .RenderPass(*renderPass)
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

    m_descriptorSet = Memory::Descriptor::Set::Builder(pool, *m_setLayout)
        .WriteBuffer(0, createInfo.particlesBuffer.DescriptorInfo().value())
        .Build();
}

void FirefliesDisplay::Init() {
    m_firefliesProgram->Init();
    m_partitionLightsProgram->Init();
}

void FirefliesDisplay::Draw(const vk::CommandBuffer &commandBuffer, bool reflected) {
    m_pipeline->Bind(commandBuffer);
    m_pipeline->BindDescriptorSet(0, commandBuffer, *m_descriptorSet);

    const Info info {
        .viewProjection = m_camera.Projection() * m_camera.View()
    };

    m_pipeline->PushConstants(commandBuffer, vk::ShaderStageFlagBits::eVertex, 0, info);

    m_mesh.Bind(commandBuffer);
    m_mesh.Draw(commandBuffer, m_particlesBuffer.InstanceCount());
}
