//
// Created by radue on 12/2/2024.
//

#include "partitionLights.h"

#include "imgui_impl_vulkan.h"
#include "../../renderer.h"
#include "compute/pipeline.h"
#include "core/shader.h"
#include "graphics/renderPass.h"
#include "memory/buffer.h"
#include "memory/image.h"
#include "memory/sampler.h"
#include "memory/descriptor/set.h"
#include "memory/descriptor/setLayout.h"
#include "renderPasses/depthPrepass.h"

PartitionLights::PartitionLights(const CreateInfo &createInfo)
    : m_chunksPerAxis(createInfo.chunksPerAxis), m_frustumBuffer(createInfo.frustumBuffer),
    m_particlesBuffer(createInfo.particlesBuffer), m_lightIndicesBuffer(createInfo.lightIndicesBuffer)
{
    m_debugImage = Memory::Image::Builder()
        .LayersCount(1)
        .MipLevels(1)
        .Format(vk::Format::eR32Sfloat)
        .Extent({ m_chunksPerAxis.x, m_chunksPerAxis.y, 1 })
        .UsageFlags(vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled)
        .SampleCount(vk::SampleCountFlagBits::e1)
        .InitialLayout(vk::ImageLayout::eGeneral)
        .Build();

    m_setLayout = Memory::Descriptor::SetLayout::Builder()
        .AddBinding(0, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute)
        .AddBinding(1, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute)
        .AddBinding(2, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute)
        .AddBinding(3, vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eCompute)
        .AddBinding(4, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eCompute)
        .Build();

    m_pipeline = Compute::Pipeline::Builder()
        .Shader("shaders/compute/partitionLights.comp")
        .DescriptorSetLayout(0, mgv::Renderer::GlobalSetLayout())
        .DescriptorSetLayout(1, *m_setLayout)
        .Build();

    ResetDescriptorSets();
}

void PartitionLights::Compute() {
    const auto currentFrame = mgv::Renderer::CurrentFrame();
    const auto commandBuffer = *currentFrame.commandBuffers.at(Core::CommandBuffer::Usage::Updates);
    m_pipeline->Bind(commandBuffer);
    m_pipeline->BindDescriptorSet(0, commandBuffer, mgv::Renderer::GlobalDescriptorSet());
    m_pipeline->BindDescriptorSet(1, commandBuffer, *m_descriptorSets[currentFrame.imageIndex]);

    commandBuffer.dispatch(m_chunksPerAxis.x, m_chunksPerAxis.y, 1);
}

void PartitionLights::ResetDescriptorSets() {
    const auto debugImageDescriptorInfo = vk::DescriptorImageInfo()
            .setSampler(Memory::Sampler::Get(vk::Filter::eNearest, vk::Filter::eNearest, vk::SamplerAddressMode::eClampToEdge, vk::SamplerMipmapMode::eNearest))
            .setImageView(m_debugImage->ImageView())
            .setImageLayout(vk::ImageLayout::eGeneral);

    m_descriptorSets.resize(mgv::Renderer::SwapChain().ImageCount());
    const auto depthPrepass = mgv::Renderer::DepthPrepass().RenderPass();
    for (uint32_t i = 0; i < mgv::Renderer::SwapChain().ImageCount(); i++) {
        const auto depthImageView = depthPrepass->OutputImage(i).ImageView();
        const auto depthDescriptorInfo = vk::DescriptorImageInfo()
            .setSampler(Memory::Sampler::Get(vk::Filter::eNearest, vk::Filter::eNearest, vk::SamplerAddressMode::eClampToEdge, vk::SamplerMipmapMode::eNearest))
            .setImageView(depthImageView)
            .setImageLayout(vk::ImageLayout::eDepthReadOnlyOptimal);

        m_descriptorSets[i] = Memory::Descriptor::Set::Builder(mgv::Renderer::DescriptorPool(), *m_setLayout)
            .WriteBuffer(0, m_frustumBuffer.DescriptorInfo())
            .WriteBuffer(1, m_particlesBuffer.DescriptorInfo())
            .WriteBuffer(2, m_lightIndicesBuffer.DescriptorInfo())
            .WriteImage(3, debugImageDescriptorInfo)
            .WriteImage(4, depthDescriptorInfo)
            .Build();
    }

    OnUIReset();
}

void PartitionLights::OnUIAttach() {
    m_debugDescriptorSet = ImGui_ImplVulkan_AddTexture(
        Memory::Sampler::Get(vk::Filter::eNearest, vk::Filter::eNearest, vk::SamplerAddressMode::eClampToEdge, vk::SamplerMipmapMode::eNearest),
        m_debugImage->ImageView(),
        static_cast<VkImageLayout>(vk::ImageLayout::eGeneral));
}

void PartitionLights::OnUIRender() {
    ImGui::Begin("Partition Lights");
    const ImVec2 size = ImGui::GetContentRegionAvail();
    ImGui::Image(m_debugDescriptorSet, ImVec2(size.x, size.x), ImVec2(0, 1), ImVec2(1, 0));
    ImGui::End();
}

void PartitionLights::OnUIReset() {
    OnUIDetach();
    OnUIAttach();
}

void PartitionLights::OnUIDetach() {
    ImGui_ImplVulkan_RemoveTexture(m_debugDescriptorSet);
}
