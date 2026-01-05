//
// Created by radue on 1/4/2026.
//
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include "dynamicRender.h"

#include <ranges>

#include "ecs/components/renderTarget.h"
#include "pipeline.h"

void Coral::Graphics::DynamicRender::Render(const Core::CommandBuffer& commandBuffer, const vk::RenderingInfo& info) const {
	commandBuffer->beginRendering(info);
	for (const auto& pipeline : m_pipelines | std::views::values) {
		pipeline->Render(commandBuffer);
	}
	commandBuffer->endRendering();
}

void Coral::Graphics::DynamicRender::AddPipeline(std::unique_ptr<Pipeline::Builder> pipelineBuilder) {
	std::unique_ptr<Pipeline> pipeline = pipelineBuilder->Build();
	m_pipelines.emplace_back(std::move(pipelineBuilder), std::move(pipeline));
}
