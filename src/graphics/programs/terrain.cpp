//
// Created by radue on 11/15/2024.
//

#include "terrain.h"

#include <random>

#include "imgui.h"
#include "imgui_impl_vulkan.h"
#include "extensions/meshShader.h"
#include "graphics/renderPass.h"
#include "memory/buffer.h"
#include "memory/sampler.h"

struct CameraData {
    glm::mat4 view;
    glm::mat4 projection;
};

Terrain::Terrain(Graphics::RenderPass &renderPass, const Memory::Descriptor::Pool &pool, const CreateInfo &createInfo)
    : Program(renderPass), m_camera(createInfo.camera)
{
    m_setLayout = Memory::Descriptor::SetLayout::Builder()
        .AddBinding(0, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eMeshEXT)
        .AddBinding(1, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment)
        .AddBinding(2, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment)
        .AddBinding(3, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eFragment)
        .AddBinding(4, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eFragment)
        .Build();

    const auto multisampleState = vk::PipelineMultisampleStateCreateInfo()
        .setRasterizationSamples(renderPass.SampleCount())
        .setSampleShadingEnable(vk::False)
        .setMinSampleShading(1.0f)
        .setPSampleMask(nullptr)
        .setAlphaToCoverageEnable(vk::False)
        .setAlphaToOneEnable(vk::False);

    constexpr auto rasterizationInfo = vk::PipelineRasterizationStateCreateInfo()
        .setDepthClampEnable(vk::False)
        .setRasterizerDiscardEnable(vk::False)
        .setPolygonMode(vk::PolygonMode::eFill)
        .setLineWidth(1.0f)
        .setCullMode(vk::CullModeFlagBits::eFront)
        .setFrontFace(vk::FrontFace::eCounterClockwise)
        .setDepthBiasEnable(vk::False)
        .setDepthBiasConstantFactor(0.0f)
        .setDepthBiasClamp(0.0f)
        .setDepthBiasSlopeFactor(0.0f);

    m_pipeline = Graphics::Pipeline::Builder()
        .AddShader(Core::Shader("shaders/terrain/terrain.mesh"))
        .AddShader(Core::Shader("shaders/terrain/terrain.frag"))
        .ColorBlendAttachment(vk::PipelineColorBlendAttachmentState()
            .setBlendEnable(vk::False)
            .setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA))
        .RenderPass(*renderPass)
        .DescriptorSetLayout(0, *m_setLayout)
        .PushConstantRange<CameraData>(vk::ShaderStageFlagBits::eMeshEXT, 0)
        .Subpass(0)
        .Multisampling(multisampleState)
        .Rasterizer(rasterizationInfo)
        .Build();

    const auto heightMapDescriptorImage = vk::DescriptorImageInfo()
        .setSampler(Memory::Sampler::Get(vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerAddressMode::eClampToEdge, vk::SamplerMipmapMode::eLinear))
        .setImageView(createInfo.heightMap.ImageView())
        .setImageLayout(vk::ImageLayout::eGeneral);

    const auto albedoDescriptorImage = vk::DescriptorImageInfo()
        .setSampler(Memory::Sampler::Get(vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerAddressMode::eClampToEdge, vk::SamplerMipmapMode::eLinear))
        .setImageView(createInfo.albedo.ImageView())
        .setImageLayout(vk::ImageLayout::eGeneral);

    const auto normalDescriptorImage = vk::DescriptorImageInfo()
        .setSampler(Memory::Sampler::Get(vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerAddressMode::eClampToEdge, vk::SamplerMipmapMode::eLinear))
        .setImageView(createInfo.normal.ImageView())
        .setImageLayout(vk::ImageLayout::eGeneral);

    m_descriptorSet = Memory::Descriptor::Set::Builder(pool, *m_setLayout)
        .WriteImage(0, heightMapDescriptorImage)
        .WriteImage(1, albedoDescriptorImage)
        .WriteImage(2, normalDescriptorImage)
        .WriteBuffer(3, createInfo.particlesBuffer.DescriptorInfo().value())
        .WriteBuffer(4, createInfo.lightIndicesBuffer.DescriptorInfo().value())
        .Build();
}

Terrain::~Terrain() = default;

void Terrain::Init() {}

void Terrain::Update(double deltaTime) {}

void Terrain::Draw(const vk::CommandBuffer &commandBuffer, const bool reflected) {
    m_pipeline->Bind(commandBuffer);
    m_pipeline->BindDescriptorSet(0, commandBuffer, *m_descriptorSet);

    const glm::mat4 cameraData[] = {
        reflected ? m_camera.FlippedView() : m_camera.View(),
        m_camera.Projection()
    };
    m_pipeline->PushConstants(commandBuffer, vk::ShaderStageFlagBits::eMeshEXT, 0, cameraData);
    Ext::MeshShader::cmdDrawMeshTasks(commandBuffer, patchCount.x , 1, patchCount.y);
}

void Terrain::InitUI() {}

void Terrain::UpdateUI() {}

void Terrain::DrawUI() {}

void Terrain::DestroyUI() {}

