//
// Created by radue on 10/24/2024.
//

#include "circleProgram.h"

CircleProgram::CircleProgram(const Core::Device &device, const Graphics::Renderer &renderer,
    const uint32_t instanceCount): m_instanceCount(instanceCount) {
    m_setLayout = Memory::Descriptor::SetLayout::Builder(device)
            .AddBinding(0, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eMeshEXT)
            .Build();

    m_vertexColorBuffer = std::make_unique<Memory::Buffer<glm::vec4>>(
        device, instanceCount,
        vk::BufferUsageFlagBits::eStorageBuffer,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

    const auto vertexColorDescriptor = m_vertexColorBuffer->DescriptorInfo();
    if (!vertexColorDescriptor.has_value()) {
        throw std::runtime_error("Failed to create vertex color buffer descriptor");
    }

    m_descriptorSet = Memory::Descriptor::Set::Builder(device, renderer.DescriptorPool(), *m_setLayout)
        .WriteBuffer(0, vertexColorDescriptor.value())
        .Build();

    m_graphicsProgram = Graphics::Pipeline::Builder()
        .AddShader(Core::Shader(device, "shaders/defaultMeshEXT/default.mesh"))
        .AddShader(Core::Shader(device, "shaders/defaultMeshEXT/default.frag"))
        .ColorBlendAttachment(vk::PipelineColorBlendAttachmentState()
            .setBlendEnable(VK_FALSE)
            .setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA))
        .RenderPass(*renderer.SwapChain().RenderPass())
        .PushConstantRange<uint32_t>(vk::ShaderStageFlagBits::eMeshEXT, 0)
        .DescriptorSetLayout(0, *m_setLayout)
        .Subpass(0)
        .Build(device);
}

void CircleProgram::Init() const {
    m_vertexColorBuffer->Map();
    for (uint32_t i = 0; i < m_instanceCount; i++) {
        constexpr auto red = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
        constexpr auto green = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
        constexpr auto blue = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
        auto color = glm::vec4(0.0f);
        if (i < m_instanceCount / 3) {
            const float t = static_cast<float>(i) / (m_instanceCount / 3);
            color = glm::mix(red, green, t);
        } else if (i < 2 * m_instanceCount / 3) {
            const float t = static_cast<float>(i - m_instanceCount / 3) / (m_instanceCount / 3);
            color = glm::mix(green, blue, t);
        } else {
            const float t = static_cast<float>(i - 2 * m_instanceCount / 3) / (m_instanceCount / 3);
            color = glm::mix(blue, red, t);
        }
        m_vertexColorBuffer->WriteAt(i, color);
    }
    m_vertexColorBuffer->Flush();
    m_vertexColorBuffer->Unmap();
}

void CircleProgram::Update() const {
    m_vertexColorBuffer->Map();

    const auto firstColor = m_vertexColorBuffer->ReadAt(0).value();
    for (uint32_t i = 0; i < m_instanceCount - 1; i++) {

        m_vertexColorBuffer->WriteAt(i, m_vertexColorBuffer->ReadAt(i + 1).value());
    }
    m_vertexColorBuffer->WriteAt(m_instanceCount - 1, firstColor);
    m_vertexColorBuffer->Flush();
    m_vertexColorBuffer->Unmap();
}

void CircleProgram::Draw(const vk::CommandBuffer &commandBuffer) const {
    m_graphicsProgram->Bind(commandBuffer);
    m_graphicsProgram->BindDescriptorSet(0, commandBuffer, *m_descriptorSet);
    m_graphicsProgram->PushConstants(commandBuffer, vk::ShaderStageFlagBits::eMeshEXT, 0, m_instanceCount);
    Ext::MeshShader::cmdDrawMeshTasks(commandBuffer, m_instanceCount, 1, 1);
}
