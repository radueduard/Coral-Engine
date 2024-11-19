//
// Created by radue on 11/17/2024.
//

#include "lake.h"

#include "extensions/meshShader.h"
#include "graphics/renderPass.h"

Lake::Lake(Core::Device &device, Graphics::RenderPass &renderPass, const Memory::Descriptor::Pool &pool, const CreateInfo &createInfo)
    : Program(renderPass), m_device(device),  m_camera(createInfo.camera)
{
    m_setLayout = Memory::Descriptor::SetLayout::Builder(device)
            .Build();

    const auto multisampleState = vk::PipelineMultisampleStateCreateInfo()
        .setRasterizationSamples(renderPass.SampleCount())
        .setSampleShadingEnable(vk::False)
        .setMinSampleShading(1.0f)
        .setPSampleMask(nullptr)
        .setAlphaToCoverageEnable(vk::False)
        .setAlphaToOneEnable(vk::False);

    m_pipeline = Graphics::Pipeline::Builder()
        .AddShader(Core::Shader(device, "shaders/terrain/lake.mesh"))
        .AddShader(Core::Shader(device, "shaders/terrain/lake.frag"))
        .ColorBlendAttachment(vk::PipelineColorBlendAttachmentState()
            .setBlendEnable(VK_FALSE)
            .setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA))
        .RenderPass(*renderPass)
        .DescriptorSetLayout(0, *m_setLayout)
        .PushConstantRange<glm::mat4>(vk::ShaderStageFlagBits::eMeshEXT, 0)
        .Subpass(0)
        .Multisampling(multisampleState)
        .Build(device);

    m_descriptorSet = Memory::Descriptor::Set::Builder(m_device, pool, *m_setLayout)
        .Build();
}

void Lake::Init() {}

void Lake::Update(double deltaTime) {}

void Lake::Draw(const vk::CommandBuffer &commandBuffer) {
    m_pipeline->Bind(commandBuffer);
    // m_pipeline->BindDescriptorSet(0, commandBuffer, *m_descriptorSet);

    const auto viewProj = m_camera.Projection() * m_camera.View();
    m_pipeline->PushConstants(commandBuffer, vk::ShaderStageFlagBits::eMeshEXT, 0, viewProj);
    Ext::MeshShader::cmdDrawMeshTasks(commandBuffer, patchCount.x , 1, patchCount.y);
}

void Lake::InitUI() {}

void Lake::UpdateUI() {}

void Lake::DrawUI() {}

void Lake::DestroyUI() {}