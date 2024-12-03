//
// Created by radue on 11/6/2024.
//

#include "displayNormals.h"

#include <queue>

#include "assets/manager.h"
#include "components/object.h"
#include "components/renderMesh.h"
#include "core/input.h"

DisplayNormals::DisplayNormals(Graphics::RenderPass &renderPass, const Memory::Descriptor::Pool &pool, const CreateInfo &createInfo)
    : Program(renderPass), m_object(createInfo.object), m_modelBuffer(createInfo.modelBuffer), m_mvpBuffer(createInfo.mvpBuffer) {
    m_setLayout = Memory::Descriptor::SetLayout::Builder()
            .AddBinding(0, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eVertex)
            .AddBinding(1, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eVertex)
            .AddBinding(2, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment)
            .AddBinding(3, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment)
            .AddBinding(4, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment)
            // .AddBinding(5, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment)
            // .AddBinding(6, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment)
            .Build();

    const auto multisampleState = vk::PipelineMultisampleStateCreateInfo()
            .setRasterizationSamples(renderPass.SampleCount())
            .setSampleShadingEnable(vk::False)
            .setMinSampleShading(1.0f)
            .setPSampleMask(nullptr)
            .setAlphaToCoverageEnable(vk::False)
            .setAlphaToOneEnable(vk::False);

    m_descriptorSet = Memory::Descriptor::Set::Builder(pool, *m_setLayout)
        .WriteBuffer(0, m_modelBuffer.DescriptorInfo().value())
        .WriteBuffer(1, m_mvpBuffer.DescriptorInfo().value())
        .WriteImage(2, Asset::Manager::GetTextureArray("baseColor").DescriptorInfo())
        .WriteImage(3, Asset::Manager::GetTextureArray("normal").DescriptorInfo())
        .WriteImage(4, Asset::Manager::GetTextureArray("metallicRoughness").DescriptorInfo())
        // .WriteImage(5, Asset::Manager::GetTextureArray("occlusion").DescriptorInfo())
        // .WriteImage(6, Asset::Manager::GetTextureArray("emissive").DescriptorInfo())
        .Build();

    auto attributes = std::unordered_set{
        mgv::Mesh::Attribute::Position,
        mgv::Mesh::Attribute::Normal,
        mgv::Mesh::Attribute::Tangent,
        mgv::Mesh::Attribute::TexCoord0,
        mgv::Mesh::Attribute::TexCoord1,
        mgv::Mesh::Attribute::Color0,
    };

    const auto attributeDescriptions = mgv::Mesh::Vertex::GetAttributeDescriptions(attributes);
    const auto bindingDescriptions = mgv::Mesh::Vertex::GetBindingDescriptions();

    m_pipeline = Graphics::Pipeline::Builder()
            .AddShader(Core::Shader("shaders/parameterShow/normal.vert"))
            .AddShader(Core::Shader("shaders/parameterShow/normal.frag"))
            .ColorBlendAttachment(vk::PipelineColorBlendAttachmentState()
                .setBlendEnable(VK_FALSE)
                .setColorWriteMask(
                    vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB |
                    vk::ColorComponentFlagBits::eA))
            .RenderPass(*renderPass)
            .DescriptorSetLayout(0, *m_setLayout)
            .PushConstantRange<uint32_t>(vk::ShaderStageFlagBits::eVertex, 0)
            .PushConstantRange<Material>(vk::ShaderStageFlagBits::eFragment, sizeof(uint32_t))
            .VertexInputState(vk::PipelineVertexInputStateCreateInfo()
                .setVertexBindingDescriptions(bindingDescriptions)
                .setVertexAttributeDescriptions(attributeDescriptions))
            .Subpass(0)
            .Multisampling(multisampleState)
            .Build();
}

void DisplayNormals::Init() {}

void DisplayNormals::Update(double deltaTime) {}

void DisplayNormals::Draw(const vk::CommandBuffer &commandBuffer, bool reflected) {
    m_pipeline->Bind(commandBuffer);
    m_pipeline->BindDescriptorSet(0, commandBuffer, *m_descriptorSet);

    std::queue<mgv::Object*> objects;
    objects.push(m_object);

    while (!objects.empty()) {
        const auto o = objects.front();
        objects.pop();

        if (const auto renderMesh = o->Get<mgv::RenderMesh>(); renderMesh.has_value()) {
            m_pipeline->PushConstants(commandBuffer, vk::ShaderStageFlagBits::eVertex, 0, o->Index());
            for (const auto &[mesh, material] : renderMesh.value()->Targets()) {
                Material displayNormalMaterial = {
                    .alphaCutoff = material->Parameters().alphaCutoff,
                    .baseColorId = material->Parameters().baseColorId,
                    .normalId = material->Parameters().normalId
                };

                m_pipeline->PushConstants(commandBuffer, vk::ShaderStageFlagBits::eFragment, sizeof(uint32_t), displayNormalMaterial);

                mesh->Bind(commandBuffer);
                mesh->Draw(commandBuffer, 1);
            }
        }

        for (const auto child : o->Children()) {
            objects.push(child);
        }
    }
}
