//
// Created by radue on 11/8/2024.
//

#pragma once

#include <memory>

#include <vulkan/vulkan.hpp>

#include "../shader/shader.h"

namespace Core {
    class Device;
}

namespace Memory::Descriptor {
    class SetLayout;
    class Set;
}

namespace Compute {

    class Pipeline {
    public:
        explicit Pipeline(Core::Shader*, std::string  kernelName = "main");
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
        Core::Shader* m_shader;
        std::string m_kernelName;

        vk::Pipeline m_pipeline;
        vk::PipelineLayout m_pipelineLayout;
    };
}
