//
// Created by radue on 12/1/2024.
//

#include "firefliesDisplay.h"

#include "assets/importer.h"
#include "renderer.h"
#include "compute/programs/fireflies.h"
#include "compute/programs/partitionLights.h"
#include "graphics/pipeline.h"
#include "graphics/renderPass.h"
#include "graphics/objects/mesh.h"
#include "memory/buffer.h"
#include "memory/descriptor/set.h"
#include "renderPasses/depthPrepass.h"
#include "renderPasses/graphicsPass.h"
#include "renderPasses/reflectionPass.h"

FirefliesDisplay::FirefliesDisplay(const CreateInfo &createInfo)
    : Program({
        mgv::Renderer::DepthPrepass().RenderPass(),
        mgv::Renderer::ReflectionPass().RenderPass(),
        mgv::Renderer::GraphicsPass().RenderPass()
    }),
    m_mesh(*mgv::Mesh::Sphere()), m_particlesBuffer(createInfo.particlesBuffer)
{
    const auto firefliesCreateInfo = Fireflies::CreateInfo {
        .particlesBuffer = createInfo.particlesBuffer,
        .trajectoriesBuffer = createInfo.trajectoriesBuffer,
        .heightMap = createInfo.heightMap,
    };

    m_firefliesProgram = std::make_unique<Fireflies>(firefliesCreateInfo);

    const auto partitionLightsCreateInfo = PartitionLights::CreateInfo {
        .chunksPerAxis = { 64, 64 },
        .particlesBuffer = createInfo.particlesBuffer,
        .lightIndicesBuffer = createInfo.lightIndicesBuffer,
    };

    m_partitionLightsProgram = std::make_unique<PartitionLights>(partitionLightsCreateInfo);

    m_setLayout = Memory::Descriptor::SetLayout::Builder()
        .AddBinding(0, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eVertex)
        .Build();

    const auto depthPrepass = mgv::Renderer::DepthPrepass().RenderPass();
    const auto reflectionPass = mgv::Renderer::ReflectionPass().RenderPass();
    const auto graphicsPass = mgv::Renderer::GraphicsPass().RenderPass();

    auto pipelineBuilder = Graphics::Pipeline::Builder();

    const auto vertexAttributeDescriptions = mgv::Mesh::Vertex::GetAttributeDescriptions({mgv::Mesh::Attribute::Position});
    const auto vertexBindingDescriptions = mgv::Mesh::Vertex::GetBindingDescriptions();

    pipelineBuilder
        .AddShader("shaders/fireflies/fireflies.vert")
        .DescriptorSetLayout(0, mgv::Renderer::GlobalSetLayout())
        .DescriptorSetLayout(1, *m_setLayout)
        .PushConstantRange<glm::uint>(vk::ShaderStageFlagBits::eVertex)
        .ColorBlendAttachment(vk::PipelineColorBlendAttachmentState()
            .setBlendEnable(vk::False)
            .setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA))
        .RenderPass(**depthPrepass)
        .Subpass(0)
        .Multisampling(vk::PipelineMultisampleStateCreateInfo()
            .setRasterizationSamples(depthPrepass->SampleCount()))
        .VertexInputState(vk::PipelineVertexInputStateCreateInfo()
            .setVertexAttributeDescriptions(vertexAttributeDescriptions)
            .setVertexBindingDescriptions(vertexBindingDescriptions));

    m_pipelines[depthPrepass] = pipelineBuilder.Build();

    pipelineBuilder
        .AddShader("shaders/fireflies/fireflies.frag")
        .RenderPass(**reflectionPass)
        .Multisampling(vk::PipelineMultisampleStateCreateInfo()
            .setRasterizationSamples(reflectionPass->SampleCount()));

    m_pipelines[reflectionPass] = pipelineBuilder.Build();

    pipelineBuilder
        .RenderPass(**graphicsPass)
        .Multisampling(vk::PipelineMultisampleStateCreateInfo()
            .setRasterizationSamples(graphicsPass->SampleCount()));

    m_pipelines[graphicsPass] = pipelineBuilder.Build();

    ResetDescriptorSets();
}

void FirefliesDisplay::Init() {
    m_firefliesProgram->Init();
    m_partitionLightsProgram->Init();
}

void FirefliesDisplay::Update(double deltaTime) {
    m_firefliesProgram->Update();
    m_partitionLightsProgram->Update();

    m_firefliesProgram->Compute();
    m_partitionLightsProgram->Compute();
}

void FirefliesDisplay::Draw(const vk::CommandBuffer &commandBuffer, const Graphics::RenderPass *renderPass) const {
    const auto reflected = renderPass == mgv::Renderer::ReflectionPass().RenderPass();
    const auto& pipeline = m_pipelines.at(renderPass);

    pipeline->Bind(commandBuffer);
    pipeline->BindDescriptorSet(0, commandBuffer, mgv::Renderer::GlobalDescriptorSet());
    pipeline->BindDescriptorSet(1, commandBuffer, *m_descriptorSet);
    pipeline->PushConstants(commandBuffer, vk::ShaderStageFlagBits::eVertex, 0, reflected ? 1u : 0u);

    m_mesh.Bind(commandBuffer);
    m_mesh.Draw(commandBuffer, m_particlesBuffer.InstanceCount());
}

void FirefliesDisplay::ResetDescriptorSets() {
    m_descriptorSet = Memory::Descriptor::Set::Builder(mgv::Renderer::DescriptorPool(), *m_setLayout)
        .WriteBuffer(0, m_particlesBuffer.DescriptorInfo())
        .Build();

    m_firefliesProgram->ResetDescriptorSets();
    m_partitionLightsProgram->ResetDescriptorSets();

    OnUIReset();
}

void FirefliesDisplay::OnUIReset() {
    m_partitionLightsProgram->OnUIReset();
}
