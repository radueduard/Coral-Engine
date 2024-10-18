//
// Created by radue on 10/17/2024.
//

#include "program.h"

#include "../core/shader.h"

namespace Graphics {
    Program::Program(const Core::Device &device, const vk::RenderPass &renderPass, const int32_t subpass)
        : m_device(device) {
        constexpr auto layoutInfo = vk::PipelineLayoutCreateInfo();
        m_layout = (*device).createPipelineLayout(layoutInfo);

        const auto colorBlendAttachment = vk::PipelineColorBlendAttachmentState()
            .setBlendEnable(vk::False)
            .setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);

        m_pipeline = Pipeline::Builder()
            .AddShader(Core::Shader(device, "shaders/defaultMeshEXT/default.mesh"))
            .AddShader(Core::Shader(device, "shaders/defaultMeshEXT/default.frag"))
            .ColorBlendAttachment(colorBlendAttachment)
            .PipelineLayout(m_layout)
            .RenderPass(renderPass)
            .Subpass(subpass)
            .Build(device);
    }

    Program::~Program() {
        (*m_device).waitIdle();
        (*m_device).destroyPipelineLayout(m_layout);
    }

    void Program::BindPipeline(const vk::CommandBuffer& commandBuffer) const {
        m_pipeline->Bind(commandBuffer);
    }

}
