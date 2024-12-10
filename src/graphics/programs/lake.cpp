//
// Created by radue on 11/17/2024.
//

#include "lake.h"

#include <random>

#include "imgui_impl_vulkan.h"
#include "renderer.h"
#include "extensions/meshShader.h"
#include "graphics/pipeline.h"
#include "graphics/renderPass.h"
#include "memory/buffer.h"
#include "memory/image.h"
#include "memory/sampler.h"
#include "memory/descriptor/set.h"
#include "memory/descriptor/setLayout.h"
#include "renderPasses/depthPrepass.h"
#include "renderPasses/graphicsPass.h"
#include "renderPasses/reflectionPass.h"
#include "utils/random.h"

Lake::Lake(const CreateInfo &createInfo)
    : Program({
        mgv::Renderer::DepthPrepass().RenderPass(),
        mgv::Renderer::GraphicsPass().RenderPass()
    }),
    m_depthOnly(createInfo.depthOnly),
    m_particlesBuffer(createInfo.particlesBuffer), m_lightIndicesBuffer(createInfo.lightIndicesBuffer)
{
    m_setLayout = Memory::Descriptor::SetLayout::Builder()
        .AddBinding(0, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment)
        .AddBinding(1, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eFragment)
        .AddBinding(2, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eFragment)
        .Build();

    const auto depthPrepass = mgv::Renderer::DepthPrepass().RenderPass();
    const auto graphicsPass = mgv::Renderer::GraphicsPass().RenderPass();

    auto pipelineBuilder = Graphics::Pipeline::Builder();

    pipelineBuilder
        .AddShader("shaders/terrain/lake.mesh")
        .ColorBlendAttachment(vk::PipelineColorBlendAttachmentState()
            .setBlendEnable(vk::False)
            .setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA))
        .RenderPass(**depthPrepass)
        .DescriptorSetLayout(0, mgv::Renderer::GlobalSetLayout())
        .DescriptorSetLayout(1, *m_setLayout)
        .Subpass(0)
        .Multisampling(vk::PipelineMultisampleStateCreateInfo()
            .setRasterizationSamples(depthPrepass->SampleCount()));

    m_pipelines[depthPrepass] = pipelineBuilder.Build();

    pipelineBuilder
        .AddShader("shaders/terrain/lake.frag")
        .RenderPass(**graphicsPass)
        .Multisampling(vk::PipelineMultisampleStateCreateInfo()
            .setRasterizationSamples(graphicsPass->SampleCount()));

    m_pipelines[graphicsPass] = pipelineBuilder.Build();

    m_spectrum = Memory::Image::Builder()
       .LayersCount(1)
       .MipLevels(1)
       .Format(vk::Format::eR16G16Sfloat)
       .Extent({ 1024, 1024, 1 })
       .UsageFlags(vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst)
       .ViewType(vk::ImageViewType::e2D)
       .Build();

    ResetDescriptorSets();
}

void Lake::Init() {
    const auto stagingBuffer = std::make_unique<Memory::Buffer>(
        sizeof(glm::u16vec2), 1024 * 1024,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

    stagingBuffer->Map<glm::u16vec2>();
    for (uint32_t i = 0; i < 1024; i++) {
        for (uint32_t j = 0; j < 1024; j++) {
            stagingBuffer->WriteAt(i * 1024 + j, glm::u16vec2 { Utils::Random::UniformIntegralValue<uint16_t>(0, UINT16_MAX), Utils::Random::UniformIntegralValue<uint16_t>(0, UINT16_MAX) });
        }
    }
    stagingBuffer->Flush();
    stagingBuffer->Unmap();

    m_spectrum->TransitionLayout(vk::ImageLayout::eTransferDstOptimal);
    m_spectrum->Copy(**stagingBuffer, 0, 0);
    m_spectrum->TransitionLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
}

void Lake::Draw(const vk::CommandBuffer &commandBuffer, const Graphics::RenderPass *renderPass) const {
    const uint32_t currentFrame = mgv::Renderer::CurrentFrame().imageIndex;
    const auto& pipeline = m_pipelines.at(renderPass);

    pipeline->Bind(commandBuffer);
    pipeline->BindDescriptorSet(0, commandBuffer, mgv::Renderer::GlobalDescriptorSet());
    pipeline->BindDescriptorSet(1, commandBuffer, *m_descriptorSets[currentFrame]);
    Ext::MeshShader::cmdDrawMeshTasks(commandBuffer, patchCount.x , 1, patchCount.y);
}

void Lake::ResetDescriptorSets() {
    const uint32_t framesInFlight = mgv::Renderer::SwapChain().ImageCount();

    m_descriptorSets.resize(framesInFlight);
    const auto reflectionPass = mgv::Renderer::ReflectionPass().RenderPass();
    for (uint32_t i = 0; i < framesInFlight; i++) {
        const auto reflectionImageInfo = vk::DescriptorImageInfo()
            .setSampler(Memory::Sampler::Get(vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerAddressMode::eClampToEdge, vk::SamplerMipmapMode::eLinear))
            .setImageView(reflectionPass->OutputImage(i).ImageView())
            .setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

        m_descriptorSets[i] = Memory::Descriptor::Set::Builder(mgv::Renderer::DescriptorPool(), *m_setLayout)
            .WriteImage(0, reflectionImageInfo)
            .WriteBuffer(1, m_particlesBuffer.DescriptorInfo())
            .WriteBuffer(2, m_lightIndicesBuffer.DescriptorInfo())
            .Build();
    }

    OnUIReset();
}

void Lake::OnUIAttach() {
    spectrumDescriptorSet = ImGui_ImplVulkan_AddTexture(
        Memory::Sampler::Get(vk::Filter::eNearest, vk::Filter::eNearest, vk::SamplerAddressMode::eRepeat, vk::SamplerMipmapMode::eLinear),
        m_spectrum->ImageView(),
        static_cast<VkImageLayout>(vk::ImageLayout::eShaderReadOnlyOptimal));

    reflectionDescriptorSets.clear();
    reflectionDescriptorSets.reserve(mgv::Renderer::SwapChain().ImageCount());
    const auto reflectionPass = mgv::Renderer::ReflectionPass().RenderPass();
    for (uint32_t i = 0; i < mgv::Renderer::SwapChain().ImageCount(); i++) {
        reflectionDescriptorSets.emplace_back(ImGui_ImplVulkan_AddTexture(
            Memory::Sampler::Get(vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerAddressMode::eClampToEdge, vk::SamplerMipmapMode::eLinear),
            reflectionPass->OutputImage(i).ImageView(),
            static_cast<VkImageLayout>(vk::ImageLayout::eShaderReadOnlyOptimal)));
    }
}

void Lake::OnUIRender() {
    ImGui::Begin("Lake");
    const ImVec2 size = ImGui::GetContentRegionAvail();
    ImGui::Image(spectrumDescriptorSet, ImVec2(size.x, size.x));

    ImGui::Separator();
    ImGui::Text("Reflection");
    const uint32_t currentFrame = mgv::Renderer::CurrentFrame().imageIndex;
    ImGui::Image(reflectionDescriptorSets[currentFrame], ImVec2(size.x, size.x));
    ImGui::End();
}

void Lake::OnUIReset() {
    OnUIDetach();
    OnUIAttach();
}

void Lake::OnUIDetach() {
    ImGui_ImplVulkan_RemoveTexture(spectrumDescriptorSet);
    for (const auto descriptorSet : reflectionDescriptorSets) {
        ImGui_ImplVulkan_RemoveTexture(descriptorSet);
    }
}