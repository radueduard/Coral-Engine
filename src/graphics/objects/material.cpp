//
// Created by radue on 11/5/2024.
//

#include "material.h"

#include "core/scheduler.h"
#include "memory/descriptor/set.h"
#include "memory/gpuStructs.h"
#include "ecs/entity.h"
#include "gui/elements/popup.h"

namespace Coral::Graphics {
    Material::Material(const Builder &builder) : m_uuid(builder.m_uuid), m_name(builder.m_name), m_textures(builder.m_textures) {
		static auto descriptorSetLayout = Memory::Descriptor::SetLayout::Builder()
			.AddBinding(0, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eFragment)				// parameters
			.AddBinding(1, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment)		// albedo texture
    		.AddBinding(2, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment)		// normal texture
    		.AddBinding(3, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment)		// metallic texture
    		.AddBinding(4, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment)		// roughness texture
    		.AddBinding(5, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment)		// emissive texture
    		.AddBinding(6, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment)		// ambient occlusion texture
			.Build();

        const GPU::Material parameters = {
            .alphaCutoff = builder.m_alphaCutoff,
            .doubleSided = builder.m_doubleSided,
            .roughnessFactor = builder.m_roughnessFactor,
            .metallicFactor = builder.m_metallicFactor,
            .emissiveFactor = builder.m_emissiveFactor,
            .baseColorFactor = builder.m_baseColorFactor,
        };

    	const auto stagingBuffer = Memory::Buffer::Builder()
    		.InstanceCount(1)
    		.InstanceSize(sizeof(GPU::Material))
    		.UsageFlags(vk::BufferUsageFlagBits::eTransferSrc)
			.MemoryProperty(vk::MemoryPropertyFlagBits::eHostVisible)
			.MemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent)
    		.Build();

    	auto writable = stagingBuffer->Map<GPU::Material>();
    	writable[0] = parameters;
    	stagingBuffer->Flush();
    	stagingBuffer->Unmap();

    	m_parametersBuffer = Memory::Buffer::Builder()
			.InstanceCount(1)
			.InstanceSize(sizeof(GPU::Material))
			.UsageFlags(vk::BufferUsageFlagBits::eUniformBuffer)
    		.UsageFlags(vk::BufferUsageFlagBits::eTransferDst)
    		.MemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal)
    		.Build();

    	m_parametersBuffer->CopyBuffer(stagingBuffer);

    	m_descriptorSet = Memory::Descriptor::Set::Builder(Core::GlobalScheduler().DescriptorPool(), *descriptorSetLayout)
    		.WriteBuffer(0, m_parametersBuffer->DescriptorInfo())
			.WriteImage(1, m_textures.at(PBR::Usage::Albedo)->DescriptorInfo())
    		.WriteImage(2, m_textures.at(PBR::Usage::Normal)->DescriptorInfo())
    		.WriteImage(3, m_textures.at(PBR::Usage::Metalic)->DescriptorInfo())
    		.WriteImage(4, m_textures.at(PBR::Usage::Roughness)->DescriptorInfo())
    		.WriteImage(5, m_textures.at(PBR::Usage::Emissive)->DescriptorInfo())
    		.WriteImage(6, m_textures.at(PBR::Usage::AmbientOcclusion)->DescriptorInfo())
			.Build();
    }
}
