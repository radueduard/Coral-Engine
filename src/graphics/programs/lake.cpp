//
// Created by radue on 11/17/2024.
//

#include "lake.h"

#include <random>

#include "imgui.h"
#include "imgui_impl_vulkan.h"
#include "extensions/meshShader.h"
#include "graphics/renderer.h"
#include "graphics/renderPass.h"
#include "memory/buffer.h"
#include "memory/sampler.h"

Lake::Lake(Graphics::RenderPass &renderPass, const Memory::Descriptor::Pool &pool, const CreateInfo &createInfo)
    : Program(renderPass), m_camera(createInfo.camera)
{
    m_setLayout = Memory::Descriptor::SetLayout::Builder()
        .AddBinding(0, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment)
        .AddBinding(1, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eMeshEXT | vk::ShaderStageFlagBits::eFragment)
        .AddBinding(2, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eFragment)
        .AddBinding(3, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eFragment)
        .Build();

    const uint32_t framesInFlight = mgv::Renderer::SwapChain().ImageCount();
    m_reflectionTextures.resize(framesInFlight);
    for (uint32_t i = 0; i < framesInFlight; i++) {
        m_reflectionTextures[i] = &createInfo.reflectionPass.OutputImage(i);
    }

    const auto multisampleState = vk::PipelineMultisampleStateCreateInfo()
        .setRasterizationSamples(renderPass.SampleCount())
        .setSampleShadingEnable(vk::False)
        .setMinSampleShading(1.0f)
        .setPSampleMask(nullptr)
        .setAlphaToCoverageEnable(vk::False)
        .setAlphaToOneEnable(vk::False);

    m_spectrum = Memory::Image::Builder()
        .LayersCount(1)
        .MipLevels(1)
        .Format(vk::Format::eR16G16Sfloat)
        .Extent({ 1024, 1024, 1 })
        .UsageFlags(vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst)
        .ViewType(vk::ImageViewType::e2D)
        .Build();

    m_cameraBuffer = std::make_unique<Memory::Buffer<CameraData>>(
        1,
        vk::BufferUsageFlagBits::eUniformBuffer,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

    m_pipeline = Graphics::Pipeline::Builder()
        .AddShader(Core::Shader("shaders/terrain/lake.mesh"))
        .AddShader(Core::Shader("shaders/terrain/lake.frag"))
        .ColorBlendAttachment(vk::PipelineColorBlendAttachmentState()
            .setBlendEnable(VK_FALSE)
            .setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA))
        .RenderPass(*renderPass)
        .DescriptorSetLayout(0, *m_setLayout)
        .Subpass(0)
        .Multisampling(multisampleState)
        .Build();

    m_descriptorSets.resize(framesInFlight);
    for (uint32_t i = 0; i < framesInFlight; i++) {
        const auto reflectionImageInfo = vk::DescriptorImageInfo()
            .setSampler(Memory::Sampler::Get(vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerAddressMode::eClampToEdge, vk::SamplerMipmapMode::eLinear))
            .setImageView(m_reflectionTextures[i]->ImageView())
            .setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

        m_descriptorSets[i] = Memory::Descriptor::Set::Builder(pool, *m_setLayout)
            .WriteImage(0, reflectionImageInfo)
            .WriteBuffer(1, m_cameraBuffer->DescriptorInfo().value())
            .WriteBuffer(2, createInfo.particlesBuffer.DescriptorInfo().value())
            .WriteBuffer(3, createInfo.lightIndicesBuffer.DescriptorInfo().value())
            .Build();
    }
}

void Lake::Init() {
    const auto stagingBuffer = std::make_unique<Memory::Buffer<glm::u16vec2>>(
        1024 * 1024,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

    stagingBuffer->Map();

    static std::mt19937 rng(std::random_device{}());
    static std::uniform_int_distribution<uint16_t> dist;

    for (uint32_t i = 0; i < 1024; i++) {
        for (uint32_t j = 0; j < 1024; j++) {
            stagingBuffer->WriteAt(i * 1024 + j, { dist(rng), dist(rng) });
        }
    }
    stagingBuffer->Flush();
    stagingBuffer->Unmap();

    m_spectrum->TransitionLayout(vk::ImageLayout::eTransferDstOptimal);
    m_spectrum->Copy(**stagingBuffer, 0, 0);
    m_spectrum->TransitionLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
}

void Lake::Update(double deltaTime) {
    m_cameraBuffer->Map();
    m_cameraBuffer->WriteAt(0, CameraData {
        .view = m_camera.View(),
        .projection = m_camera.Projection(),
        .inverseView = m_camera.InverseView(),
        .inverseProjection = m_camera.InverseProjection(),
        .flippedView = m_camera.FlippedView(),
        .flippedInverseView = m_camera.FlippedInverseView()
    });
    m_cameraBuffer->Flush();
    m_cameraBuffer->Unmap();
}

void Lake::Draw(const vk::CommandBuffer &commandBuffer, const bool reflected) {
    const uint32_t currentFrame = mgv::Renderer::CurrentFrame().imageIndex;

    m_pipeline->Bind(commandBuffer);
    m_pipeline->BindDescriptorSet(0, commandBuffer, *m_descriptorSets[currentFrame]);

    Ext::MeshShader::cmdDrawMeshTasks(commandBuffer, patchCount.x , 1, patchCount.y);
}

void Lake::InitUI() {
    spectrumDescriptorSet = ImGui_ImplVulkan_AddTexture(
        Memory::Sampler::Get(vk::Filter::eNearest, vk::Filter::eNearest, vk::SamplerAddressMode::eRepeat, vk::SamplerMipmapMode::eLinear),
        m_spectrum->ImageView(),
        static_cast<VkImageLayout>(vk::ImageLayout::eShaderReadOnlyOptimal));

    for (const auto & m_reflectionTexture : m_reflectionTextures) {
        reflectionDescriptorSets.emplace_back(ImGui_ImplVulkan_AddTexture(
            Memory::Sampler::Get(vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerAddressMode::eClampToEdge, vk::SamplerMipmapMode::eLinear),
            m_reflectionTexture->ImageView(),
            static_cast<VkImageLayout>(vk::ImageLayout::eShaderReadOnlyOptimal)));
    }

}

void Lake::UpdateUI() {}

void Lake::DrawUI() {
    ImGui::Begin("Lake");
    const ImVec2 size = ImGui::GetContentRegionAvail();
    ImGui::Image(spectrumDescriptorSet, ImVec2(size.x, size.x));

    ImGui::Separator();
    ImGui::Text("Reflection");
    const uint32_t currentFrame = mgv::Renderer::CurrentFrame().imageIndex;
    ImGui::Image(reflectionDescriptorSets[currentFrame], ImVec2(size.x, size.x));

    ImGui::End();
}

void Lake::DestroyUI() {
    ImGui_ImplVulkan_RemoveTexture(spectrumDescriptorSet);
    for (const auto descriptorSet : reflectionDescriptorSets) {
        ImGui_ImplVulkan_RemoveTexture(descriptorSet);
    }
}