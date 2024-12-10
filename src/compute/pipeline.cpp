//
// Created by radue on 11/8/2024.
//

#include "pipeline.h"

#include <iostream>

#include "core/device.h"

#include "memory/descriptor/setLayout.h"
#include "memory/descriptor/set.h"

namespace Compute {
    Pipeline::Builder & Pipeline::Builder::Shader(const std::string& path) {
        m_shaderModule = std::make_unique<Core::Shader>(path, vk::ShaderStageFlagBits::eCompute);
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

    std::unique_ptr<Pipeline> Pipeline::Builder::Build() {
        const auto pipelineLayoutInfo = vk::PipelineLayoutCreateInfo()
            .setSetLayouts(m_descriptorSetLayouts)
            .setPushConstantRanges(m_pushConstantRanges);

        m_pipelineLayout = (*Core::Device::Get()).createPipelineLayout(pipelineLayoutInfo);

        m_shaderStage = vk::PipelineShaderStageCreateInfo()
            .setStage(vk::ShaderStageFlagBits::eCompute)
            .setModule(**m_shaderModule)
            .setPName("main");

        return std::make_unique<Pipeline>(*this);
    }

    Pipeline::Pipeline(const Builder& builder) : m_pipelineLayout(builder.m_pipelineLayout)
    {
        const auto createInfo = vk::ComputePipelineCreateInfo()
            .setLayout(m_pipelineLayout)
            .setStage(builder.m_shaderStage)
            .setBasePipelineHandle(builder.m_basePipeline)
            .setBasePipelineIndex(builder.m_basePipelineIndex);

        const auto pipeline = (*Core::Device::Get()).createComputePipeline(nullptr, createInfo);
        if (pipeline.result != vk::Result::eSuccess) {
            std::cerr << "Failed to create compute pipeline" << std::endl;
        }
        m_pipeline = pipeline.value;
    }

    Pipeline::~Pipeline() {
        const auto &device = Core::Device::Get();
        (*device).waitIdle();
        (*device).destroyPipeline(m_pipeline);
        (*device).destroyPipelineLayout(m_pipelineLayout);
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
