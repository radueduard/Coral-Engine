//
// Created by radue on 11/8/2024.
//

#include "pipeline.h"

#include <iostream>

namespace Compute {
    Pipeline::Builder & Pipeline::Builder::Shader(const Core::Shader &shader) {
        m_shaderModule = *shader;
        return *this;
    }

    Pipeline::Builder& Pipeline::Builder::BasePipeline(const vk::Pipeline& basePipeline, int32_t basePipelineIndex) {
        m_basePipeline = basePipeline;
        m_basePipelineIndex = basePipelineIndex;
        return *this;
    }

    Pipeline::Builder& Pipeline::Builder::DescriptorSetLayout(const uint32_t setNumber, const Memory::Descriptor::SetLayout& layout) {
        if (m_descriptorSetLayouts.size() != setNumber) {
            std::cerr << "Descriptor set layouts must be added in order, and without gaps" << std::endl;
            return *this;
        }
        m_descriptorSetLayouts.emplace_back(*layout);
        return *this;
    }

    Pipeline::Builder& Pipeline::Builder::DescriptorSetLayouts(uint32_t startingSet, const std::vector<Memory::Descriptor::SetLayout>& layouts) {
        for (const auto &layout : layouts) {
            DescriptorSetLayout(startingSet++, layout);
        }
        return *this;
    }

    std::unique_ptr<Pipeline> Pipeline::Builder::Build(const Core::Device &device) {
        const auto pipelineLayoutInfo = vk::PipelineLayoutCreateInfo()
            .setSetLayouts(m_descriptorSetLayouts)
            .setPushConstantRanges(m_pushConstantRanges);

        m_pipelineLayout = (*device).createPipelineLayout(pipelineLayoutInfo);

        m_shaderStage = vk::PipelineShaderStageCreateInfo()
            .setStage(vk::ShaderStageFlagBits::eCompute)
            .setModule(m_shaderModule)
            .setPName("main");

        return std::make_unique<Pipeline>(device, *this);
    }

    Pipeline::Pipeline(const Core::Device& device, const Builder& builder)
        : m_device(device), m_pipelineLayout(builder.m_pipelineLayout), m_shaderModule(builder.m_shaderModule)
    {
        const auto createInfo = vk::ComputePipelineCreateInfo()
            .setLayout(m_pipelineLayout)
            .setStage(builder.m_shaderStage)
            .setBasePipelineHandle(builder.m_basePipeline)
            .setBasePipelineIndex(builder.m_basePipelineIndex);

        const auto pipeline = (*device).createComputePipeline(nullptr, createInfo);
        if (pipeline.result != vk::Result::eSuccess) {
            std::cerr << "Failed to create compute pipeline" << std::endl;
        }
        m_pipeline = pipeline.value;
    }

    Pipeline::~Pipeline() {
        (*m_device).waitIdle();
        (*m_device).destroyPipeline(m_pipeline);
        (*m_device).destroyPipelineLayout(m_pipelineLayout);
    }

    void Pipeline::Bind(const vk::CommandBuffer commandBuffer) const {
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, m_pipeline);
    }

    void Pipeline::BindDescriptorSet(const uint32_t setNumber, const vk::CommandBuffer commandBuffer, const Memory::Descriptor::Set & descriptorSet) const {
        commandBuffer.bindDescriptorSets(
            vk::PipelineBindPoint::eCompute,
            m_pipelineLayout,
            setNumber,
            *descriptorSet,
            nullptr);
    }

    void Pipeline::BindDescriptorSets(const uint32_t startingSet, const vk::CommandBuffer commandBuffer, const std::vector<Memory::Descriptor::Set> & descriptorSets) const {
        std::vector<vk::DescriptorSet> sets;
        for (const auto &descriptorSet : descriptorSets) {
            sets.push_back(*descriptorSet);
        }

        commandBuffer.bindDescriptorSets(
            vk::PipelineBindPoint::eCompute,
            m_pipelineLayout,
            startingSet,
            sets,
            nullptr);
    }
}
