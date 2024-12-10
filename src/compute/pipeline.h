//
// Created by radue on 11/8/2024.
//

#pragma once

#include <memory>

#include <vulkan/vulkan.hpp>

#include "core/shader.h"

namespace Memory::Descriptor {
    class SetLayout;
    class Set;
}

namespace Compute {

    class Pipeline {
    public:
        class Builder {
            friend class Pipeline;
        public:
            Builder() = default;
            ~Builder() = default;

            Builder(const Builder &) = delete;
            Builder &operator=(const Builder &) = delete;

            Builder &Shader(const std::string &path);
            Builder &BasePipeline(const vk::Pipeline &, int32_t);

            template<typename T>
            Builder &PushConstantRange(const vk::ShaderStageFlags &stageFlags, const uint32_t offset = 0) {
                const auto pushConstantRange = vk::PushConstantRange()
                    .setStageFlags(stageFlags)
                    .setOffset(offset)
                    .setSize(sizeof(T));
                m_pushConstantRanges.push_back(pushConstantRange);
                return *this;
            }

            Builder &DescriptorSetLayout(uint32_t setNumber, const Memory::Descriptor::SetLayout &layout);
            Builder &DescriptorSetLayouts(uint32_t startingSet, const std::vector<Memory::Descriptor::SetLayout> &layouts);

            std::unique_ptr<Pipeline> Build();

        private:

            std::vector<vk::PushConstantRange> m_pushConstantRanges;
            std::vector<vk::DescriptorSetLayout> m_descriptorSetLayouts;

            vk::PipelineLayout m_pipelineLayout;
            std::unique_ptr<Core::Shader> m_shaderModule;
            vk::PipelineShaderStageCreateInfo m_shaderStage;

            vk::Pipeline m_basePipeline = nullptr;
            int32_t m_basePipelineIndex = -1;
        };
        Pipeline(const Builder& builder);
        ~Pipeline();

        Pipeline(const Pipeline &) = delete;
        Pipeline &operator=(const Pipeline &) = delete;

        void Bind(vk::CommandBuffer) const;
        template<typename T>
        void PushConstants(const vk::CommandBuffer commandBuffer, const vk::ShaderStageFlags stageFlags, const uint32_t offset, const T& data) const {
            commandBuffer.pushConstants(
                m_pipelineLayout,
                stageFlags,
                offset,
                sizeof(T),
                &data);
        }

        void BindDescriptorSet(uint32_t, vk::CommandBuffer, const Memory::Descriptor::Set &) const;
        void BindDescriptorSets(uint32_t, vk::CommandBuffer, const std::vector<Memory::Descriptor::Set> &) const;
    private:
        vk::Pipeline m_pipeline;
        vk::PipelineLayout m_pipelineLayout;
    };
}
