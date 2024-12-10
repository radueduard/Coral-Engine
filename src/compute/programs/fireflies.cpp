//
// Created by radue on 12/1/2024.
//

#include "fireflies.h"

#include "renderer.h"
#include "compute/pipeline.h"
#include "core/shader.h"
#include "core/window.h"
#include "memory/buffer.h"
#include "memory/image.h"
#include "memory/sampler.h"
#include "memory/descriptor/set.h"
#include "memory/descriptor/setLayout.h"

Fireflies::Fireflies(const CreateInfo &createInfo)
    : m_heightMap(createInfo.heightMap), m_particlesBuffer(createInfo.particlesBuffer) {
    m_setLayout = Memory::Descriptor::SetLayout::Builder()
            .AddBinding(0, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute)
            .AddBinding(1, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eCompute)
            .Build();

    m_pipeline = Compute::Pipeline::Builder()
            .Shader("shaders/compute/fireflies.comp")
            .DescriptorSetLayout(0, *m_setLayout)
            .PushConstantRange<glm::vec2>(vk::ShaderStageFlagBits::eCompute)
            .Build();

    ResetDescriptorSets();
}

void Fireflies::Compute() {
    const auto commandBuffer = *mgv::Renderer::CurrentFrame().commandBuffers.at(Core::CommandBuffer::Usage::Updates);
    m_pipeline->Bind(commandBuffer);
    m_pipeline->BindDescriptorSet(0, commandBuffer, *m_descriptorSet);

    const glm::vec2 times = { Core::Window::TimeElapsed(), Core::Window::DeltaTime() };
    m_pipeline->PushConstants(commandBuffer, vk::ShaderStageFlagBits::eCompute, 0, times);
    const uint32_t totalInstances = m_particlesBuffer.InstanceCount() / 64;
    commandBuffer.dispatch(totalInstances, 1, 1);
}

void Fireflies::ResetDescriptorSets() {
    const auto noiseDescriptorInfo = vk::DescriptorImageInfo()
        .setSampler(Memory::Sampler::Get(vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerAddressMode::eRepeat, vk::SamplerMipmapMode::eLinear))
        .setImageView(m_heightMap.ImageView())
        .setImageLayout(vk::ImageLayout::eGeneral);

    m_descriptorSet = Memory::Descriptor::Set::Builder(mgv::Renderer::DescriptorPool(), *m_setLayout)
        .WriteBuffer(0, m_particlesBuffer.DescriptorInfo())
        .WriteImage(1, noiseDescriptorInfo)
        .Build();
}