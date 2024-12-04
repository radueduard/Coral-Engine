//
// Created by radue on 11/29/2024.
//

#include "generateTerrain.h"

#include <random>
#include <glm/glm.hpp>

#include "imgui_impl_vulkan.h"
#include "graphics/renderer.h"
#include "memory/sampler.h"

GenerateTerrain::GenerateTerrain(const Memory::Descriptor::Pool &pool, const CreateInfo &createInfo) {
    m_noiseTextureCount = static_cast<uint32_t>(glm::log2(static_cast<float>(createInfo.size))) + 1;

    m_setLayout = Memory::Descriptor::SetLayout::Builder()
        .AddBinding(0, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eCompute)
        .AddBinding(1, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eCompute)
        .AddBinding(2, vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eCompute)
        .AddBinding(3, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eCompute)
        .AddBinding(4, vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eCompute)
        .AddBinding(5, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eCompute)
        .AddBinding(6, vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eCompute)
        .Build();

    m_pipeline = Compute::Pipeline::Builder()
        .Shader(Core::Shader("shaders/compute/generateTerrain.comp"))
        .DescriptorSetLayout(0, *m_setLayout)
        .Build();

    m_settingsBuffer = std::make_unique<Memory::Buffer<Settings>>(
        1,
        vk::BufferUsageFlagBits::eUniformBuffer,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

    m_noiseTextures = Memory::Image::Builder()
        .LayersCount(1)
        .MipLevels(m_noiseTextureCount)
        .Format(vk::Format::eR8G8B8A8Srgb)
        .Extent({ createInfo.size, createInfo.size, 1 })
        .UsageFlags(vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled)
        .SampleCount(vk::SampleCountFlagBits::e1)
        .Build();

    m_heightMap = Memory::Image::Builder()
        .LayersCount(1)
        .MipLevels(1)
        .Format(vk::Format::eR32Sfloat)
        .Extent({ createInfo.size, createInfo.size, 1 })
        .UsageFlags(vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled)
        .SampleCount(vk::SampleCountFlagBits::e1)
        .InitialLayout(vk::ImageLayout::eGeneral)
        .Build();

    m_albedo = Memory::Image::Builder()
        .LayersCount(1)
        .MipLevels(1)
        .Format(vk::Format::eR32G32B32A32Sfloat)
        .Extent({ createInfo.size, createInfo.size, 1 })
        .UsageFlags(vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled)
        .SampleCount(vk::SampleCountFlagBits::e1)
        .InitialLayout(vk::ImageLayout::eGeneral)
        .Build();

    m_normal = Memory::Image::Builder()
        .LayersCount(1)
        .MipLevels(1)
        .Format(vk::Format::eR8G8B8A8Unorm)
        .Extent({ createInfo.size, createInfo.size, 1 })
        .UsageFlags(vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled)
        .SampleCount(vk::SampleCountFlagBits::e1)
        .InitialLayout(vk::ImageLayout::eGeneral)
        .Build();

    const auto noiseDescriptorInfo = vk::DescriptorImageInfo()
        .setSampler(Memory::Sampler::Get(vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerAddressMode::eRepeat, vk::SamplerMipmapMode::eLinear))
        .setImageView(m_noiseTextures->ImageView())
        .setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

    const auto heightMapDescriptorInfo = vk::DescriptorImageInfo()
        .setSampler(Memory::Sampler::Get(vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerAddressMode::eClampToEdge, vk::SamplerMipmapMode::eLinear))
        .setImageView(m_heightMap->ImageView())
        .setImageLayout(vk::ImageLayout::eGeneral);

    const auto albedoDescriptorInfo = vk::DescriptorImageInfo()
        .setSampler(Memory::Sampler::Get(vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerAddressMode::eClampToEdge, vk::SamplerMipmapMode::eLinear))
        .setImageView(m_albedo->ImageView())
        .setImageLayout(vk::ImageLayout::eGeneral);

    const auto normalDescriptorInfo = vk::DescriptorImageInfo()
        .setSampler(Memory::Sampler::Get(vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerAddressMode::eClampToEdge, vk::SamplerMipmapMode::eLinear))
        .setImageView(m_normal->ImageView())
        .setImageLayout(vk::ImageLayout::eGeneral);

    m_descriptorSet = Memory::Descriptor::Set::Builder(pool, *m_setLayout)
        .WriteBuffer(0, m_settingsBuffer->DescriptorInfo().value())
        .WriteImage(1, noiseDescriptorInfo)
        .WriteImage(2, heightMapDescriptorInfo)
        .WriteImage(3, createInfo.albedoTextures.DescriptorInfo())
        .WriteImage(4, albedoDescriptorInfo)
        .WriteImage(5, createInfo.normalTextures.DescriptorInfo())
        .WriteImage(6, normalDescriptorInfo)
        .Build();
}

void GenerateTerrain::Init() {
    m_noiseTextures->TransitionLayout(vk::ImageLayout::eTransferDstOptimal);

    boost::random::mt19937 rng;
    const boost::random::uniform_int_distribution<uint32_t> dist(0, 255);

    auto size = m_noiseTextures->Extent().width;
    for (uint32_t i = 0; i < m_noiseTextureCount; i++) {
        const auto stagingBuffer = std::make_unique<Memory::Buffer<glm::u8vec4>>(
            size * size,
            vk::BufferUsageFlagBits::eTransferSrc,
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

        stagingBuffer->Map();
        for (uint32_t j = 0; j < size * size; j++) {
            uint32_t value = dist(rng);
            stagingBuffer->WriteAt(j, { value, value, value, 255 });
        }
        stagingBuffer->Flush();
        stagingBuffer->Unmap();

        m_noiseTextures->Copy(**stagingBuffer, i);
        size /= 2;
    }

    m_noiseTextures->TransitionLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

    m_settings.maxHeight = 5.f;
    m_settings.lodCount = m_noiseTextureCount;
    for (uint32_t i = 0; i < m_noiseTextureCount; i++) {
        m_settings.noiseWeights[i] = 1.f / pow(2.f, static_cast<float>(m_noiseTextureCount - i - 2));
    }

    m_settingsBuffer->Map();
    m_settingsBuffer->Write(&m_settings);
    m_settingsBuffer->Unmap();


}

void GenerateTerrain::Update() {
}

void GenerateTerrain::Compute(const vk::CommandBuffer &commandBuffer) {
    if (!m_changed) {
        return;
    }

    m_pipeline->Bind(commandBuffer);
    m_pipeline->BindDescriptorSet(0, commandBuffer, *m_descriptorSet);
    commandBuffer.dispatch(m_noiseTextures->Extent().width / 32, m_noiseTextures->Extent().height / 32, 1);

    m_changed = false;
}

void GenerateTerrain::InitUI() {
    m_noiseTextureViews = m_noiseTextures->IndividualMipLevels();

    for (uint32_t i = 0; i < m_noiseTextureCount; i++) {
        m_uiTextureDescriptors.emplace_back(ImGui_ImplVulkan_AddTexture(
            Memory::Sampler::Get(vk::Filter::eNearest, vk::Filter::eNearest, vk::SamplerAddressMode::eRepeat, vk::SamplerMipmapMode::eLinear),
            m_noiseTextureViews[i],
            static_cast<VkImageLayout>(vk::ImageLayout::eShaderReadOnlyOptimal)));
    }

    m_heightMapDescriptorSet = ImGui_ImplVulkan_AddTexture(
        Memory::Sampler::Get(vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerAddressMode::eClampToEdge, vk::SamplerMipmapMode::eLinear),
        m_heightMap->ImageView(),
        static_cast<VkImageLayout>(vk::ImageLayout::eGeneral));

    m_albedoDescriptorSet = ImGui_ImplVulkan_AddTexture(
        Memory::Sampler::Get(vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerAddressMode::eClampToEdge, vk::SamplerMipmapMode::eLinear),
        m_albedo->ImageView(),
        static_cast<VkImageLayout>(vk::ImageLayout::eGeneral));

    m_normalDescriptorSet = ImGui_ImplVulkan_AddTexture(
        Memory::Sampler::Get(vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerAddressMode::eClampToEdge, vk::SamplerMipmapMode::eLinear),
        m_normal->ImageView(),
        static_cast<VkImageLayout>(vk::ImageLayout::eGeneral));
}

void GenerateTerrain::UpdateUI() {
    if (m_changed) {
        m_settingsBuffer->Map();
        m_settingsBuffer->Write(&m_settings);
        m_settingsBuffer->Unmap();
    }
}

void GenerateTerrain::DrawUI() {
    ImGui::Begin("Terrain");

    m_changed |= ImGui::DragFloat("Max Height", &m_settings.maxHeight, 0.1f, 0.0f, 100.0f);

    ImGui::Text("Noise Textures:");

    const ImVec2 size = ImGui::GetContentRegionAvail();
    ImGui::SliderInt("Selected mip level", reinterpret_cast<int*>(&m_selectedMipLevel), 0, static_cast<int>(m_noiseTextureCount - 1));
    ImGui::Image(m_uiTextureDescriptors[m_selectedMipLevel], ImVec2(size.x, size.x), ImVec2(0, 1), ImVec2(1, 0));
    m_changed |= ImGui::DragFloat("Noise Weight", &m_settings.noiseWeights[m_selectedMipLevel], 0.01f, 0.0f, 1.0f);

    ImGui::Separator();
    ImGui::Text("Height Map:");
    ImGui::Image(m_heightMapDescriptorSet, ImVec2(size.x, size.x), ImVec2(0, 1), ImVec2(1, 0));

    ImGui::Separator();
    ImGui::Text("Albedo:");
    ImGui::Image(m_albedoDescriptorSet, ImVec2(size.x, size.x), ImVec2(0, 1), ImVec2(1, 0));

    ImGui::Separator();
    ImGui::Text("Normal:");
    ImGui::Image(m_normalDescriptorSet, ImVec2(size.x, size.x), ImVec2(0, 1), ImVec2(1, 0));

    ImGui::End();
}

void GenerateTerrain::DestroyUI() {
    for (const auto &descriptor : m_uiTextureDescriptors) {
        ImGui_ImplVulkan_RemoveTexture(descriptor);
    }
    for (const auto &view : m_noiseTextureViews) {
        (*Core::Device::Get()).destroyImageView(view);
    }
    ImGui_ImplVulkan_RemoveTexture(m_heightMapDescriptorSet);
    ImGui_ImplVulkan_RemoveTexture(m_albedoDescriptorSet);
    ImGui_ImplVulkan_RemoveTexture(m_normalDescriptorSet);
}
