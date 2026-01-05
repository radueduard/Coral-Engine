//
// Created by radue on 1/4/2026.
//

#pragma once
#include <vulkan/vulkan.hpp>

#include "pipeline.h"

namespace Coral::Core {
	class CommandBuffer;
}
namespace Coral::Graphics {
	class DynamicRender {
	public:
		DynamicRender() = default;
		~DynamicRender() = default;

		void Render(const Core::CommandBuffer& commandBuffer,const vk::RenderingInfo& info) const;

		void AddPipeline(std::unique_ptr<Pipeline::Builder> pipelineBuilder);

	private:
		std::vector<std::pair<std::unique_ptr<Pipeline::Builder>, std::unique_ptr<Pipeline>>> m_pipelines;
	};
}
