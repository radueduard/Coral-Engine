//
// Created by radue on 12/20/2025.
//

#include "noise.h"

#include "compute/pipeline.h"
#include "core/scheduler.h"
#include "memory/imageView.h"
#include "shader/manager.h"

Coral::Utils::PerlinNoise2D::PerlinNoise2D(Math::Vector2u size, u32 octaves) {
	m_image = Memory::Image::Builder()
		  .Format(vk::Format::eR8Unorm)
		  .Extent(Math::Vector2u { size.x, size.y })
		  .UsageFlags(vk::ImageUsageFlagBits::eStorage)
		  .UsageFlags(vk::ImageUsageFlagBits::eSampled)
		  .InitialLayout(vk::ImageLayout::eGeneral)
		  .Build();

	auto outputImageView = Memory::ImageView::Builder(*m_image).ViewType(vk::ImageViewType::e2D).Build();
	auto outputImageInfo = vk::DescriptorImageInfo()
		.setImageLayout(vk::ImageLayout::eGeneral)
		.setImageView(**outputImageView);

	const auto* shader = Shader::Manager::Get().GetShader("perlin2D", "generate");
	const auto computePipeline = Compute::Pipeline(*shader);

	const auto set =
		Memory::Descriptor::Set::Builder(Context::Scheduler().DescriptorPool(), computePipeline.DescriptorSetLayout(0))
			.WriteImage(0, outputImageInfo)
			.Build();

	Context::Device().RunSingleTimeCommand(
		[&](const Core::CommandBuffer& commandBuffer) {
			computePipeline.Bind(commandBuffer);
			computePipeline.BindDescriptorSet(0, commandBuffer, *set);

			const struct PushConstants {
				u32 octaves;
			} pushConstants {
				.octaves = octaves
			};

			computePipeline.PushConstants(commandBuffer, vk::ShaderStageFlagBits::eCompute, 0, pushConstants);

			const auto groupSize = Math::Vector2u { 16u, 16u };
			const Math::Vector2u dispatchSize = (size + (groupSize - 1u)) / groupSize;
			commandBuffer->dispatch(dispatchSize.x, dispatchSize.y, 1);

			m_image->TransitionLayout(commandBuffer, vk::ImageLayout::eShaderReadOnlyOptimal);
		},
		vk::QueueFlagBits::eCompute);
}

Coral::Utils::PerlinNoise3D::PerlinNoise3D(Math::Vector3u size, u32 octaves) : m_size(size) {
	const auto stagingImage = Memory::Image::Builder()
		.Format(vk::Format::eR8Unorm)
		.Extent(size)
		.MipLevels(octaves)
		.UsageFlags(vk::ImageUsageFlagBits::eStorage)
		.InitialLayout(vk::ImageLayout::eGeneral)
		.Build();

	m_image = Memory::Image::Builder()
		  .Format(vk::Format::eR8Unorm)
		  .Extent(size)
		  .MipLevels(octaves)
		  .UsageFlags(vk::ImageUsageFlagBits::eStorage)
		  .UsageFlags(vk::ImageUsageFlagBits::eSampled)
		  .InitialLayout(vk::ImageLayout::eGeneral)
		  .Build();

	const auto generateImageView = Memory::ImageView::Builder(*stagingImage)
		.ViewType(vk::ImageViewType::e3D)
		.Build();

	const auto finalImageView = Memory::ImageView::Builder(*m_image)
		.ViewType(vk::ImageViewType::e3D)
		.Build();

	const auto* generateNoiseShader = Shader::Manager::Get().GetShader("perlin3D", "generate");
	const auto generateNoisePipeline = Compute::Pipeline(*generateNoiseShader);

	const auto* blurShader = Shader::Manager::Get().GetShader("blur", "Gaussian3D");
	const auto blurPipeline = Compute::Pipeline(*blurShader);

	const auto generateNoiseSet =
		Memory::Descriptor::Set::Builder(Context::Scheduler().DescriptorPool(), generateNoisePipeline.DescriptorSetLayout(0))
			.WriteImage(0,
				vk::DescriptorImageInfo()
					.setImageLayout(vk::ImageLayout::eGeneral)
					.setImageView(**generateImageView))
			.Build();

	const auto blurSet =
		Memory::Descriptor::Set::Builder(Context::Scheduler().DescriptorPool(), blurPipeline.DescriptorSetLayout(0))
			.WriteImage(0,
				vk::DescriptorImageInfo()
					.setImageLayout(vk::ImageLayout::eGeneral)
					.setImageView(**generateImageView))
			.WriteImage(1,
				vk::DescriptorImageInfo()
					.setImageLayout(vk::ImageLayout::eGeneral)
					.setImageView(**finalImageView))
			.Build();

	Context::Device().RunSingleTimeCommand(
		[&](const Core::CommandBuffer& commandBuffer) {
			generateNoisePipeline.Bind(commandBuffer);
			generateNoisePipeline.BindDescriptorSet(0, commandBuffer, *generateNoiseSet);

			const struct PushConstants {
				u32 octaves;
			} pushConstants {
				.octaves = octaves
			};

			generateNoisePipeline.PushConstants(commandBuffer, vk::ShaderStageFlagBits::eCompute, 0, pushConstants);

			const auto groupSize = Math::Vector3u{8u, 8u, 8u};
			const Math::Vector3u dispatchSize = (size + (groupSize - 1u)) / groupSize;
			commandBuffer->dispatch(dispatchSize.x, dispatchSize.y, dispatchSize.z);

			const auto imageMemoryBarrier = vk::ImageMemoryBarrier()
				.setSrcAccessMask(vk::AccessFlagBits::eShaderWrite)
				.setDstAccessMask(vk::AccessFlagBits::eShaderRead)
				.setOldLayout(vk::ImageLayout::eGeneral)
				.setNewLayout(vk::ImageLayout::eGeneral)
				.setImage(**stagingImage)
				.setSubresourceRange(vk::ImageSubresourceRange()
					.setAspectMask(vk::ImageAspectFlagBits::eColor)
					.setBaseMipLevel(0)
					.setLevelCount(VK_REMAINING_MIP_LEVELS)
					.setBaseArrayLayer(0)
					.setLayerCount(VK_REMAINING_ARRAY_LAYERS));

			commandBuffer->pipelineBarrier(
				vk::PipelineStageFlagBits::eComputeShader,
				vk::PipelineStageFlagBits::eComputeShader,
				{},
				{},
				{},
				{ imageMemoryBarrier }
			);

			const struct {
				alignas(16) Math::Vector3f direction;
				alignas(4) i32 kernelSize;
			} blurPayload {
				.direction = Math::Vector3f { 1.f, 0.f, 0.f },
				.kernelSize = 5
			};

			blurPipeline.Bind(commandBuffer);
			blurPipeline.BindDescriptorSet(0, commandBuffer, *blurSet);
			blurPipeline.PushConstants(commandBuffer, vk::ShaderStageFlagBits::eCompute, 0, blurPayload);
			commandBuffer->dispatch(dispatchSize.x, dispatchSize.y, dispatchSize.z);

			m_image->TransitionLayout(commandBuffer, vk::ImageLayout::eShaderReadOnlyOptimal);
		},
		vk::QueueFlagBits::eCompute);
}
