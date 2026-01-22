//
// Created by radue on 1/1/2026.
//

#include "bitonicMergeSort.h"

#include <magic_enum/magic_enum.hpp>

#include "core/scheduler.h"

Coral::Compute::BitonicMergeSorter::BitonicMergeSorter(const std::vector<u32>& elements) {
	m_pipeline = std::make_unique<Pipeline>(*Context::ShaderManager().SlangShader("sort", "bitonicSort"));
	m_buffer = Memory::Buffer::Builder()
		.InstanceCount(static_cast<u32>(elements.size()))
		.InstanceSize(sizeof(u32))
		.UsageFlags(vk::BufferUsageFlagBits::eStorageBuffer)
		.UsageFlags(vk::BufferUsageFlagBits::eTransferSrc)
		.UsageFlags(vk::BufferUsageFlagBits::eTransferDst)
		.MemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal)
		.Data(elements.data(), elements.size() * sizeof(u32))
		.Build();
}

std::vector<Coral::u32> Coral::Compute::BitonicMergeSorter::operator()() const {
	const auto descriptorSet = Memory::Descriptor::Set::Builder(Context::Scheduler().DescriptorPool(), m_pipeline->DescriptorSetLayout(0))
		.WriteBuffer(1, m_buffer->DescriptorInfo())
		.Build();

	Context::Device().RunSingleTimeCommand(
		[&](const Core::CommandBuffer& commandBuffer) {
			m_pipeline->Bind(commandBuffer);
			m_pipeline->BindDescriptorSet(0, commandBuffer, *descriptorSet);

			enum class Algorithm : u32 {
				LocalBitonicMergeSort = 0,
				LocalDisperse = 1,
				BigFlip = 2,
				BigDisperse = 3
			};

			struct PushConstants {
				alignas(4) Algorithm algorithm;
				alignas(4) u32 h;
			};

			const u32 n = m_buffer->InstanceCount();
			constexpr u32 workGroupSize = 64;
			const u32 workGroupCount = n / (workGroupSize * 2);

			auto dispatch = [&](const u32 h, const Algorithm& algorithm) {
				const PushConstants pushConstants {
					.algorithm = algorithm,
					.h = h
				};
				m_pipeline->PushConstants(commandBuffer, vk::ShaderStageFlagBits::eCompute, 0, pushConstants);
				commandBuffer->dispatch(workGroupCount, 1, 1);

				const auto bufferMemoryBarrier = vk::BufferMemoryBarrier()
					.setSrcAccessMask(vk::AccessFlagBits::eShaderWrite)
					.setDstAccessMask(vk::AccessFlagBits::eShaderRead)
					.setBuffer(**m_buffer)
					.setSize(vk::WholeSize);

				commandBuffer->pipelineBarrier(
					vk::PipelineStageFlagBits::eComputeShader,
					vk::PipelineStageFlagBits::eComputeShader,
					{},
					{},
					{ bufferMemoryBarrier },
					{});
			};

			u32 h = workGroupSize * 2;
			dispatch(h, Algorithm::LocalBitonicMergeSort);
			h *= 2;
			for (; h <= n; h *= 2) {
				dispatch(h, Algorithm::BigFlip);
				for (u32 hh = h; hh > 1; hh /= 2) {
					if (hh <= workGroupSize * 2) {
						dispatch(hh, Algorithm::LocalDisperse);
						break;
					}
					dispatch(hh, Algorithm::BigDisperse);
				}
			}
		}, vk::QueueFlagBits::eCompute);

	std::vector<u32> sortedData(m_buffer->InstanceCount());
	const auto readBackBuffer = Memory::Buffer::Builder()
		.InstanceCount(m_buffer->InstanceCount())
		.InstanceSize(sizeof(u32))
		.UsageFlags(vk::BufferUsageFlagBits::eTransferDst)
		.MemoryProperty(vk::MemoryPropertyFlagBits::eHostVisible)
		.MemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent)
		.Data(sortedData.data(), sortedData.size() * sizeof(u32))
		.Build();

	readBackBuffer->CopyBuffer(*m_buffer);
	auto mapped = readBackBuffer->Map<u32>();
	std::ranges::copy(mapped, sortedData.begin());
	readBackBuffer->Unmap();
	return sortedData;
}
