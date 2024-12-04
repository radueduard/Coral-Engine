//
// Created by radue on 12/1/2024.
//

#include "fireflies.h"

#include "memory/sampler.h"

Fireflies::Fireflies(const Memory::Descriptor::Pool &pool, const CreateInfo &createInfo)
    : m_particlesBuffer(createInfo.particlesBuffer) {
    m_setLayout = Memory::Descriptor::SetLayout::Builder()
        .AddBinding(0, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute)
        .AddBinding(1, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eCompute)
        .Build();

    m_pipeline = Compute::Pipeline::Builder()
        .Shader(Core::Shader("shaders/compute/fireflies.comp"))
        .DescriptorSetLayout(0, *m_setLayout)
        .PushConstantRange<glm::vec2>(vk::ShaderStageFlagBits::eCompute)
        .Build();

    const auto noiseDescriptorInfo = vk::DescriptorImageInfo()
        .setSampler(Memory::Sampler::Get(vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerAddressMode::eRepeat, vk::SamplerMipmapMode::eLinear))
        .setImageView(createInfo.heightMap->ImageView())
        .setImageLayout(vk::ImageLayout::eGeneral);

    m_descriptorSet = Memory::Descriptor::Set::Builder(pool, *m_setLayout)
        .WriteBuffer(0, createInfo.particlesBuffer.DescriptorInfo().value())
        .WriteImage(1, noiseDescriptorInfo)
        .Build();
}

void Fireflies::Init() {}

void Fireflies::Update() {}

void Fireflies::Compute(const vk::CommandBuffer &commandBuffer) {
    m_pipeline->Bind(commandBuffer);
    m_pipeline->BindDescriptorSet(0, commandBuffer, *m_descriptorSet);

    const glm::vec2 times = { Core::Window::TimeElapsed(), Core::Window::DeltaTime() };
    m_pipeline->PushConstants(commandBuffer, vk::ShaderStageFlagBits::eCompute, 0, times);
    const uint32_t totalInstances = m_particlesBuffer.InstanceCount() / 64;
    commandBuffer.dispatch(totalInstances, 1, 1);
}

void Fireflies::InitUI() {
}

void Fireflies::UpdateUI() {
}

void Fireflies::DrawUI() {
}

void Fireflies::DestroyUI() {
}
