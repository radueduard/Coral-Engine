//
// Created by radue on 11/29/2024.
//

#include "skyBox.h"

#include "extensions/meshShader.h"
#include "graphics/renderPass.h"

struct CameraData {
    glm::mat4 view;
    glm::mat4 projection;
};

SkyBox::SkyBox(Graphics::RenderPass &renderPass, const Memory::Descriptor::Pool &pool, const CreateInfo &createInfo)
    : Program(renderPass), m_camera(createInfo.camera)
{
    m_setLayout = Memory::Descriptor::SetLayout::Builder()
        .AddBinding(0, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment)
        .Build();

    const auto multisampleState = vk::PipelineMultisampleStateCreateInfo()
        .setRasterizationSamples(renderPass.SampleCount())
        .setSampleShadingEnable(vk::False)
        .setMinSampleShading(1.0f)
        .setPSampleMask(nullptr)
        .setAlphaToCoverageEnable(vk::False)
        .setAlphaToOneEnable(vk::False);

    m_pipeline = Graphics::Pipeline::Builder()
        .AddShader(Core::Shader("shaders/skyBox/skyBox.mesh"))
        .AddShader(Core::Shader("shaders/skyBox/skyBox.frag"))
        .ColorBlendAttachment(vk::PipelineColorBlendAttachmentState()
            .setBlendEnable(vk::False)
            .setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA))
        .RenderPass(*renderPass)
        .DescriptorSetLayout(0, *m_setLayout)
        .PushConstantRange<CameraData>(vk::ShaderStageFlagBits::eMeshEXT, 0)
        .Subpass(0)
        .Multisampling(multisampleState)
        .DepthStencil(vk::PipelineDepthStencilStateCreateInfo()
            .setDepthTestEnable(vk::False))
        .Build();

    m_descriptorSet = Memory::Descriptor::Set::Builder(pool, *m_setLayout)
        .WriteImage(0, createInfo.cubeMap.DescriptorInfo())
        .Build();

}

void SkyBox::Draw(const vk::CommandBuffer &commandBuffer, const bool reflected) {
    m_pipeline->Bind(commandBuffer);
    m_pipeline->BindDescriptorSet(0, commandBuffer, *m_descriptorSet);

    const CameraData cameraData {
        .view = reflected ? m_camera.FlippedView() : m_camera.View(),
        .projection = m_camera.Projection()
    };

    m_pipeline->PushConstants(commandBuffer, vk::ShaderStageFlagBits::eMeshEXT, 0, cameraData);
    Ext::MeshShader::cmdDrawMeshTasks(commandBuffer, 1, 1, 1);
}
