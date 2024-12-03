//
// Created by radue on 12/2/2024.
//

#include "partitionLights.h"

PartitionLights::PartitionLights(const Memory::Descriptor::Pool &pool, const CreateInfo &createInfo)
    : m_chunksPerAxis(createInfo.chunksPerAxis) {
    m_setLayout = Memory::Descriptor::SetLayout::Builder()
        .AddBinding(0, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute)
        .AddBinding(1, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute)
        .Build();

    m_pipeline = Compute::Pipeline::Builder()
        .Shader(Core::Shader("shaders/compute/partitionLights.comp"))
        .DescriptorSetLayout(0, *m_setLayout)
        .Build();

    m_lightIndicesBuffer = std::make_unique<Memory::Buffer<Indices>>(
        createInfo.chunksPerAxis * createInfo.chunksPerAxis,
        vk::BufferUsageFlagBits::eStorageBuffer,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

    m_descriptorSet = Memory::Descriptor::Set::Builder(pool, *m_setLayout)
        .WriteBuffer(0, createInfo.particlesBuffer.DescriptorInfo().value())
        .WriteBuffer(1, m_lightIndicesBuffer->DescriptorInfo().value())
        .Build();
}

void PartitionLights::Init() {
    m_lightIndicesBuffer->Map();
    for (uint32_t i = 0; i < m_chunksPerAxis * m_chunksPerAxis; i++) {
        m_lightIndicesBuffer->WriteAt(i, Indices{});
    }
    m_lightIndicesBuffer->Flush();
    m_lightIndicesBuffer->Unmap();
}

void PartitionLights::Update() {
}

void PartitionLights::Compute(const vk::CommandBuffer &commandBuffer) {
    m_pipeline->Bind(commandBuffer);
    m_pipeline->BindDescriptorSet(0, commandBuffer, *m_descriptorSet);
    commandBuffer.dispatch(m_chunksPerAxis, m_chunksPerAxis, 1);
}
