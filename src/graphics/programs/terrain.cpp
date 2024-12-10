//
// Created by radue on 11/15/2024.
//

#include "terrain.h"

#include <random>

#include "renderer.h"
#include "extensions/meshShader.h"
#include "graphics/pipeline.h"
#include "graphics/renderPass.h"
#include "memory/buffer.h"
#include "memory/image.h"
#include "memory/sampler.h"
#include "memory/descriptor/set.h"
#include "memory/descriptor/setLayout.h"
#include "renderPasses/depthPrepass.h"
#include "renderPasses/graphicsPass.h"
#include "renderPasses/reflectionPass.h"

Terrain::Terrain(const CreateInfo &createInfo)
    : Program({
        mgv::Renderer::DepthPrepass().RenderPass(),
        mgv::Renderer::ReflectionPass().RenderPass(),
        mgv::Renderer::GraphicsPass().RenderPass()
    }),
    m_heightMap(createInfo.heightMap), m_albedo(createInfo.albedo), m_normal(createInfo.normal),
    m_particlesBuffer(createInfo.particlesBuffer), m_lightIndicesBuffer(createInfo.lightIndicesBuffer)
{
    m_setLayout = Memory::Descriptor::SetLayout::Builder()
        .AddBinding(0, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eMeshEXT)
        .AddBinding(1, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment)
        .AddBinding(2, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment)
        .AddBinding(3, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eFragment)
        .AddBinding(4, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eFragment)
        .Build();

    const auto depthPrepass = mgv::Renderer::DepthPrepass().RenderPass();
    const auto reflectionPass = mgv::Renderer::ReflectionPass().RenderPass();
    const auto graphicsPass = mgv::Renderer::GraphicsPass().RenderPass();

    auto pipelineBuilder = Graphics::Pipeline::Builder();

    pipelineBuilder
        .AddShader("shaders/terrain/terrain.mesh")
        .ColorBlendAttachment(vk::PipelineColorBlendAttachmentState()
            .setBlendEnable(vk::False)
            .setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA))
        .RenderPass(**depthPrepass)
        .DescriptorSetLayout(0, mgv::Renderer::GlobalSetLayout())
        .DescriptorSetLayout(1, *m_setLayout)
        .PushConstantRange<glm::uint>(vk::ShaderStageFlagBits::eMeshEXT | vk::ShaderStageFlagBits::eFragment)
        .Subpass(0)
        .Multisampling(vk::PipelineMultisampleStateCreateInfo()
            .setRasterizationSamples(depthPrepass->SampleCount()));

    m_pipelines[depthPrepass] = pipelineBuilder.Build();

    pipelineBuilder
        .AddShader("shaders/terrain/terrain.frag")
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

void Terrain::Draw(const vk::CommandBuffer &commandBuffer, const Graphics::RenderPass *renderPass) const {
    const auto reflected = renderPass == mgv::Renderer::ReflectionPass().RenderPass();

    const auto& pipeline = m_pipelines.at(renderPass);
    pipeline->Bind(commandBuffer);
    pipeline->BindDescriptorSet(0, commandBuffer, mgv::Renderer::GlobalDescriptorSet());
    pipeline->BindDescriptorSet(1, commandBuffer, *m_descriptorSet);
    pipeline->PushConstants(commandBuffer, vk::ShaderStageFlagBits::eMeshEXT | vk::ShaderStageFlagBits::eFragment, 0, reflected ? 1u : 0u);
    Ext::MeshShader::cmdDrawMeshTasks(commandBuffer, patchCount.x , 1, patchCount.y);
}

void Terrain::ResetDescriptorSets() {
     const auto heightMapDescriptorImage = vk::DescriptorImageInfo()
        .setSampler(Memory::Sampler::Get(vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerAddressMode::eClampToEdge, vk::SamplerMipmapMode::eLinear))
        .setImageView(m_heightMap.ImageView())
        .setImageLayout(vk::ImageLayout::eGeneral);

    const auto albedoDescriptorImage = vk::DescriptorImageInfo()
        .setSampler(Memory::Sampler::Get(vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerAddressMode::eClampToEdge, vk::SamplerMipmapMode::eLinear))
        .setImageView(m_albedo.ImageView())
        .setImageLayout(vk::ImageLayout::eGeneral);

    const auto normalDescriptorImage = vk::DescriptorImageInfo()
        .setSampler(Memory::Sampler::Get(vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerAddressMode::eClampToEdge, vk::SamplerMipmapMode::eLinear))
        .setImageView(m_normal.ImageView())
        .setImageLayout(vk::ImageLayout::eGeneral);

    m_descriptorSet = Memory::Descriptor::Set::Builder(mgv::Renderer::DescriptorPool(), *m_setLayout)
        .WriteImage(0, heightMapDescriptorImage)
        .WriteImage(1, albedoDescriptorImage)
        .WriteImage(2, normalDescriptorImage)
        .WriteBuffer(3, m_particlesBuffer.DescriptorInfo())
        .WriteBuffer(4, m_lightIndicesBuffer.DescriptorInfo())
        .Build();

    OnUIReset();
}

