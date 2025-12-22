//
// Created by radue on 12/16/2025.
//
#pragma once

#include <map>
#include <memory>
#include <variant>

#include <vulkan/vulkan.hpp>

#include "core/scheduler.h"
#include "memory/descriptor/set.h"
#include "pipeline.h"

namespace Coral::Reef {
	class ComputeProgramTemplate;
}
namespace Coral::Compute {
	struct Binding {
		uint32_t set;
		uint32_t binding;

		auto operator<=>(const Binding& other) const {
			if (const auto cmp = set <=> other.set; cmp != 0) {
				return cmp;
			}
			return binding <=> other.binding;
		}
	};

	struct Resource {
		std::variant<std::shared_ptr<class Buffer>, std::shared_ptr<class Image>, std::shared_ptr<class Sampler>> resource;
		std::variant<vk::DescriptorBufferInfo, vk::DescriptorImageInfo> descriptorInfo;
	};

	class Program {
		friend class Reef::ComputeProgramTemplate;
	public:
		explicit Program(const Shader::Shader& shader) : m_shader(shader) {
			m_pipeline = std::make_unique<Compute::Pipeline>(shader);
		}
		~Program() {
			m_descriptorSets.clear();
			m_pipeline.reset();
		}

		void SetResource(const Binding& binding, const Resource& resource) {
			m_resources[binding] = resource;
		}

		void Update();
		void Build();
		void Run(const Core::CommandBuffer& commandBuffer) const;

		[[nodiscard]] const Pipeline& Pipeline() const { return *m_pipeline; }
		[[nodiscard]] Compute::Pipeline& Pipeline() { return *m_pipeline; }

		[[nodiscard]] const Shader::Shader& Shader() const { return m_shader; }

	private:
		const Shader::Shader& m_shader;
		std::unique_ptr<Compute::Pipeline> m_pipeline;

		Math::Vector3u m_groupCount;

		std::map<Binding, Resource> m_resources;
		std::vector<std::unique_ptr<Memory::Descriptor::Set>> m_descriptorSets;
	};
}