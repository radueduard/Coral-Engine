//
// Created by radue on 11/8/2024.
//

#pragma once

#include <ranges>


#include "core/device.h"
#include "shader/shader.h"

namespace Coral::Core {
	class Device;
}

namespace Coral::Memory::Descriptor {
    class SetLayout;
    class Set;
}

namespace Coral::Compute {
    class Pipeline {
    public:
        explicit Pipeline(const Shader::Shader& shader);
		~Pipeline();

    	void Create();
		void Destroy();

        Pipeline(const Pipeline &) = delete;
        Pipeline &operator=(const Pipeline &) = delete;


        void Bind(const Core::CommandBuffer&) const;
        template<typename T>
        void PushConstants(const Core::CommandBuffer& commandBuffer, const vk::ShaderStageFlags stageFlags, const uint32_t offset, const T& data) const {
            commandBuffer->pushConstants(
                m_pipelineLayout,
                stageFlags,
                offset,
                sizeof(T),
                &data);
        }

        void BindDescriptorSet(uint32_t, const Core::CommandBuffer&, const Memory::Descriptor::Set &) const;
        void BindDescriptorSets(uint32_t, const Core::CommandBuffer&, const std::vector<Memory::Descriptor::Set> &) const;

    	const Shader::Shader& GetShader() const { return m_shader; }
    	const Memory::Descriptor::SetLayout& DescriptorSetLayout(const uint32_t index) const { return *m_descriptorSetLayouts.at(index); }
		std::vector<const Memory::Descriptor::SetLayout*> DescriptorSetLayouts() const {
			return m_descriptorSetLayouts
				| std::views::transform([](const auto& layout) { return layout.get(); })
				| std::ranges::to<std::vector<const Memory::Descriptor::SetLayout*>>();
		}


    private:
        const Shader::Shader& m_shader;

        vk::Pipeline m_pipeline;
        vk::PipelineLayout m_pipelineLayout;
    	std::vector<std::unique_ptr<Memory::Descriptor::SetLayout>> m_descriptorSetLayouts;
    };
}
