//
// Created by radue on 11/8/2024.
//

#include "calculateMvp.h"

#include <queue>

CalculateMVP::CalculateMVP(Core::Device &device, const Memory::Descriptor::Pool &pool) {
    m_cameraBuffer = std::make_unique<Memory::Buffer<mgv::Camera::Info>>(
            device, 1,
            vk::BufferUsageFlagBits::eUniformBuffer,
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

    m_parentBuffer = std::make_unique<Memory::Buffer<glm::uint>>(
        device, 4096,
        vk::BufferUsageFlagBits::eUniformBuffer,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

    m_modelBuffer = std::make_unique<Memory::Buffer<glm::mat4>>(
        device, 4096,
        vk::BufferUsageFlagBits::eStorageBuffer,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

    m_outModelBuffer = std::make_unique<Memory::Buffer<glm::mat4>>(
        device, 4096,
        vk::BufferUsageFlagBits::eStorageBuffer,
        vk::MemoryPropertyFlagBits::eDeviceLocal);

    m_mvpBuffer = std::make_unique<Memory::Buffer<glm::mat4>>(
        device, 4096,
        vk::BufferUsageFlagBits::eStorageBuffer,
        vk::MemoryPropertyFlagBits::eDeviceLocal);

    m_setLayout = Memory::Descriptor::SetLayout::Builder(device)
        .AddBinding(0, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eCompute)
        .AddBinding(1, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eCompute)
        .AddBinding(2, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute)
        .AddBinding(3, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute)
        .AddBinding(4, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute)
        .Build();

    m_descriptorSet = Memory::Descriptor::Set::Builder(device, pool, *m_setLayout)
        .WriteBuffer(0, m_cameraBuffer->DescriptorInfo().value())
        .WriteBuffer(1, m_parentBuffer->DescriptorInfo().value())
        .WriteBuffer(2, m_modelBuffer->DescriptorInfo().value())
        .WriteBuffer(3, m_outModelBuffer->DescriptorInfo().value())
        .WriteBuffer(4, m_mvpBuffer->DescriptorInfo().value())
        .Build();

    m_computeProgram = Compute::Pipeline::Builder()
        .Shader(Core::Shader(device, "shaders/compute/calculateMvp.comp"))
        .DescriptorSetLayout(0, *m_setLayout)
        .Build(device);
}

void CalculateMVP::Init(const mgv::Scene &scene) const {
    const auto cameraInfo = scene.Camera().Get<mgv::Camera>().value()->BufferData();
    m_cameraBuffer->Map();
    m_cameraBuffer->Write(&cameraInfo);
    m_cameraBuffer->Unmap();

    m_modelBuffer->Map();
    m_parentBuffer->Map();

    for (uint32_t i = 0; i < 4096; i++) {
        m_modelBuffer->WriteAt(i, glm::mat4(1.0f));
        m_parentBuffer->WriteAt(i, 0);
    }

    std::queue<mgv::Object*> objects;
    objects.push(scene.Root().get());
    uint32_t index = 0;
    while (!objects.empty()) {
        const auto object = objects.front();
        objects.pop();

        object->Index() = index;
        m_modelBuffer->WriteAt(index, object->Matrix());
        m_parentBuffer->WriteAt(index, object->Parent() != nullptr ? object->Parent()->Index() : 0);

        for (const auto child : object->Children()) {
            objects.push(child);
        }
        index++;
    }
    m_modelBuffer->Unmap();
    m_parentBuffer->Unmap();
}

void CalculateMVP::Update(const mgv::Scene &scene) const {
    const auto camera = scene.Camera().Get<mgv::Camera>().value();
    // if (camera->Moved()) {
        const auto cameraInfo = camera->BufferData();
        m_cameraBuffer->Map();
        m_cameraBuffer->Write(&cameraInfo);
        m_cameraBuffer->Unmap();
    // }
    //
    // m_modelBuffer->Map();
    // m_parentBuffer->Map();
    // std::queue<mgv::Object*> objects;
    // objects.push(scene.Root().get());
    // while (!objects.empty()) {
    //     const auto object = objects.front();
    //     objects.pop();
    //
    //     if (object->Moved()) {
    //         m_modelBuffer->WriteAt(object->Index(), object->Matrix());
    //     }
    //
    //     for (const auto child : object->Children()) {
    //         objects.push(child);
    //     }
    // }
    // m_modelBuffer->Unmap();
    // m_parentBuffer->Unmap();
}

void CalculateMVP::Compute(const vk::CommandBuffer &commandBuffer) const {
    m_computeProgram->Bind(commandBuffer);
    m_computeProgram->BindDescriptorSet(0, commandBuffer, *m_descriptorSet);
    // TODO: Find better number of groups
    commandBuffer.dispatch(2, 2, 1);
}
