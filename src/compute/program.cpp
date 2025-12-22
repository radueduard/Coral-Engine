//
// Created by radue on 12/16/2025.
//

#include "program.h"


void Coral::Compute::Program::Update() {
	if (m_shader.HasReloaded()) {
		m_descriptorSets.clear();
		m_pipeline->Destroy();
		m_pipeline->Create();
		Build();
	}
}
void Coral::Compute::Program::Build() {
	std::vector<Memory::Descriptor::Set::Builder> setBuilders;
	for (const auto& descriptor : m_pipeline->DescriptorSetLayouts()) {
		setBuilders.emplace_back(Context::Scheduler().DescriptorPool(), *descriptor);
	}

	for (const auto& [binding, resource] : m_resources) {
		auto& builder = setBuilders[binding.set];
		std::visit(
			[&]<typename T>(T&& res) {
				if constexpr (std::is_same_v<T, std::shared_ptr<Buffer>>) {
					builder.WriteBuffer(binding.binding, std::get<vk::DescriptorBufferInfo>(resource.descriptorInfo));
				}
				else if constexpr (std::is_same_v<T, std::shared_ptr<Image>>) {
					builder.WriteImage(binding.binding, std::get<vk::DescriptorImageInfo>(resource.descriptorInfo));
				}
				else if constexpr (std::is_same_v<T, std::shared_ptr<Sampler>>) {
					builder.WriteImage(binding.binding, std::get<vk::DescriptorImageInfo>(resource.descriptorInfo));
				}
			},
			resource.resource);
	}
}

void Coral::Compute::Program::Run(const Core::CommandBuffer& commandBuffer) const {
	m_pipeline->Bind(commandBuffer);
	for (size_t i = 0; i < m_descriptorSets.size(); i++) {
		m_pipeline->BindDescriptorSet(static_cast<uint32_t>(i), commandBuffer, *m_descriptorSets[i]);
	}
	commandBuffer->dispatch(m_groupCount.x, m_groupCount.y, m_groupCount.z);
}
