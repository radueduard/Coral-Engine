//
// Created by radue on 11/15/2024.
//

#include "terrain.h"

#include "imgui.h"
#include "imgui_impl_vulkan.h"
#include "extensions/meshShader.h"
#include "graphics/renderPass.h"
#include "memory/buffer.h"
#include "memory/sampler.h"

Terrain::Terrain(Core::Device &device, Graphics::RenderPass &renderPass, const Memory::Descriptor::Pool &pool, const CreateInfo &createInfo)
    : Program(renderPass), m_device(device),  m_camera(createInfo.camera), m_noiseTextureCount(createInfo.noiseTextureCount)
{
    m_setLayout = Memory::Descriptor::SetLayout::Builder(device)
            .AddBinding(0, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eMeshEXT)
            .AddBinding(1, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment)
            .Build();

    const auto multisampleState = vk::PipelineMultisampleStateCreateInfo()
        .setRasterizationSamples(renderPass.SampleCount())
        .setSampleShadingEnable(vk::False)
        .setMinSampleShading(1.0f)
        .setPSampleMask(nullptr)
        .setAlphaToCoverageEnable(vk::False)
        .setAlphaToOneEnable(vk::False);

    m_pipeline = Graphics::Pipeline::Builder()
        .AddShader(Core::Shader(device, "shaders/terrain/terrain.mesh"))
        .AddShader(Core::Shader(device, "shaders/terrain/terrain.frag"))
        .ColorBlendAttachment(vk::PipelineColorBlendAttachmentState()
            .setBlendEnable(VK_FALSE)
            .setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA))
        .RenderPass(*renderPass)
        .DescriptorSetLayout(0, *m_setLayout)
        .PushConstantRange<glm::mat4>(vk::ShaderStageFlagBits::eMeshEXT, 0)
        .Subpass(0)
        .Multisampling(multisampleState)
        .Build(device);

    m_noiseTextures = Memory::Image::Builder()
        .LayersCount(1)
        .MipLevels(m_noiseTextureCount)
        .Format(vk::Format::eR8G8B8A8Srgb)
        .Extent({ 1024, 1024, 1 })
        .UsageFlags(vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled)
        .SampleCount(vk::SampleCountFlagBits::e1)
        .Build(m_device);

    const auto descriptorInfo = vk::DescriptorImageInfo()
        .setSampler(Memory::Sampler::Get(m_device, vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerAddressMode::eRepeat, vk::SamplerMipmapMode::eLinear))
        .setImageView(m_noiseTextures->ImageView())
        .setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

    m_albedoTextures = mgv::TextureArray::Builder()
        .Name("Albedo")
        .Format(vk::Format::eR8G8B8A8Srgb)
        .ImageSize(2048)
        .AddImagePath("models/textures/wavy_sand/wavy-sand_albedo.png")
        .AddImagePath("models/textures/stylized_grass/stylized-grass1_albedo.png")
        .AddImagePath("models/textures/stylized_cliff/stylized-cliff1-albedo.png")
        .AddImagePath("models/textures/snow_packed/snow-packed12-Base_Color.png")
        .CreateMipmaps()
        .Build(m_device);

    m_descriptorSet = Memory::Descriptor::Set::Builder(m_device, pool, *m_setLayout)
        .WriteImage(0, descriptorInfo)
        .WriteImage(1, m_albedoTextures->DescriptorInfo())
        .Build();
}

Terrain::~Terrain() {}

void Terrain::Init() {
    m_noiseTextures->TransitionLayout(vk::ImageLayout::eTransferDstOptimal);

    boost::random::mt19937 rng;
    const boost::random::uniform_int_distribution<uint8_t> dist(0, 255);

    auto size = 1024u;
    for (uint32_t i = 0; i < m_noiseTextureCount; i++) {
        const auto stagingBuffer = std::make_unique<Memory::Buffer<glm::u8vec4>>(
            m_device,
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
}

void Terrain::Update(double deltaTime) {}

void Terrain::Draw(const vk::CommandBuffer &commandBuffer) {
    m_pipeline->Bind(commandBuffer);
    m_pipeline->BindDescriptorSet(0, commandBuffer, *m_descriptorSet);

    const auto viewProj = m_camera.Projection() * m_camera.View();
    m_pipeline->PushConstants(commandBuffer, vk::ShaderStageFlagBits::eMeshEXT, 0, viewProj);
    Ext::MeshShader::cmdDrawMeshTasks(commandBuffer, patchCount.x , 1, patchCount.y);
}

void Terrain::InitUI() {
    m_noiseTextureViews = m_noiseTextures->IndividualMipLevels();

    for (uint32_t i = 0; i < m_noiseTextureCount; i++) {
        m_uiTextureDescriptors.emplace_back(ImGui_ImplVulkan_AddTexture(
            Memory::Sampler::Get(m_device, vk::Filter::eNearest, vk::Filter::eNearest, vk::SamplerAddressMode::eRepeat, vk::SamplerMipmapMode::eLinear),
            m_noiseTextureViews[i],
            static_cast<VkImageLayout>(vk::ImageLayout::eShaderReadOnlyOptimal)));
    }
}

void Terrain::UpdateUI() {}

void Terrain::DrawUI() {
    ImGui::Begin("Terrain");
    ImGui::DragInt2("Patch Count", &patchCount.x, 10.0f, 10, 200);

    ImGui::Text("Noise Textures:");
    const ImVec2 size = ImGui::GetContentRegionAvail();
    ImGui::SliderInt("Selected mip level", reinterpret_cast<int*>(&m_selectedMipLevel), 0, static_cast<int>(m_noiseTextureCount - 1));
    ImGui::Image(m_uiTextureDescriptors[m_selectedMipLevel], ImVec2(size.x, size.x), ImVec2(0, 1), ImVec2(1, 0));
    ImGui::End();
}

void Terrain::DestroyUI() {
    for (const auto &descriptor : m_uiTextureDescriptors) {
        ImGui_ImplVulkan_RemoveTexture(descriptor);
    }
    for (const auto &view : m_noiseTextureViews) {
        (*m_device).destroyImageView(view);
    }
}

