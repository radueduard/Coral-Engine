//
// Created by radue on 11/8/2024.
//

#include "pipeline.h"

#include <iostream>
#include <utility>
#include <ranges>

#include "core/device.h"

#include "memory/descriptor/setLayout.h"
#include "memory/descriptor/set.h"

namespace Coral::Compute {
    Pipeline::Pipeline(Shader::Shader* shader, std::string kernelName) : m_shader(shader), m_kernelName(std::move(kernelName)) {
        std::vector<Memory::Descriptor::SetLayout::Builder> setLayoutBuilders;
        for (const auto &[set, binding, name, type, count] : m_shader->Descriptors()) {
            if (setLayoutBuilders.size() <= set) {
                setLayoutBuilders.resize(set + 1);
            }
            setLayoutBuilders[set].AddBinding(binding, type, vk::ShaderStageFlagBits::eCompute, count);
        }
        
        std::vector<std::unique_ptr<Memory::Descriptor::SetLayout>> setLayouts;
        for (const auto &layoutBuilder : setLayoutBuilders) {
            setLayouts.emplace_back(layoutBuilder.Build());
        }

        std::vector<vk::DescriptorSetLayout> layouts = setLayouts
            | std::views::transform([](const auto &layout) { return **layout; })
            | std::ranges::to<std::vector<vk::DescriptorSetLayout>>();

        std::vector<vk::PushConstantRange> pushConstantRanges;
        for (const auto &[size, offset, name] : m_shader->PushConstantRanges()) {
            pushConstantRanges.emplace_back(vk::PushConstantRange()
                .setOffset(offset)
                .setSize(size)
                .setStageFlags(vk::ShaderStageFlagBits::eCompute));
        }

        const auto pipelineLayoutCreateInfo = vk::PipelineLayoutCreateInfo()
            .setSetLayouts(layouts)
            .setPushConstantRanges(pushConstantRanges);

        m_pipelineLayout = Context::Device()->createPipelineLayout(pipelineLayoutCreateInfo);

        const auto shaderStage = vk::PipelineShaderStageCreateInfo()
            .setStage(vk::ShaderStageFlagBits::eCompute)
            .setModule(**m_shader)
            .setPName(m_kernelName.c_str());

        const auto createInfo = vk::ComputePipelineCreateInfo()
            .setLayout(m_pipelineLayout)
            .setStage(shaderStage);

        const auto pipeline = Context::Device()->createComputePipeline(nullptr, createInfo);
        if (pipeline.result != vk::Result::eSuccess) {
            std::cerr << "Failed to create compute pipeline" << std::endl;
        }
        m_pipeline = pipeline.value;
    }

    Pipeline::~Pipeline() {
        Context::Device()->waitIdle();
        Context::Device()->destroyPipeline(m_pipeline);
        Context::Device()->destroyPipelineLayout(m_pipelineLayout);
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
