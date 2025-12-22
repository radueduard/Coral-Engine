//
// Created by radue on 12/20/2025.
//

#include "noise.h"

#include "compute/pipeline.h"
#include "core/scheduler.h"
#include "memory/imageView.h"
#include "shader/manager.h"

Coral::Utils::PerlinNoise3D::PerlinNoise3D(Math::Vector3u size, u32 octaves) : m_size(size) {
	auto directionSize = Math::Vector3u(pow(2, octaves));
	auto directionsNoise = DirectionsNoise<3, 3>(directionSize, octaves);

	auto inputImageViews = std::vector<std::unique_ptr<Memory::ImageView>> {};

	for (u32 i = 0; i < directionsNoise.Image().MipLevels(); i++) {
		inputImageViews.emplace_back(
			Memory::ImageView::Builder(directionsNoise.Image())
				.ViewType(vk::ImageViewType::e3D)
				.BaseMipLevel(i)
				.LevelCount(1)
				.Build());
	}

	auto inputImageInfos = std::vector<vk::DescriptorImageInfo> {};
	for (const auto& imageView : inputImageViews) {
		inputImageInfos.emplace_back(
			vk::DescriptorImageInfo()
				.setImageLayout(vk::ImageLayout::eGeneral)
				.setImageView(**imageView));
	}

	m_image = Memory::Image::Builder()
		  .Format(vk::Format::eR8Unorm)
		  .Extent(size)
		  .MipLevels(octaves)
		  .UsageFlags(vk::ImageUsageFlagBits::eStorage)
		  .UsageFlags(vk::ImageUsageFlagBits::eSampled)
		  .InitialLayout(vk::ImageLayout::eGeneral)
		  .Build();

	auto outputImageView = Memory::ImageView::Builder(*m_image).ViewType(vk::ImageViewType::e3D).Build();

	const auto* shader = Shader::Manager::Get().GetShader("perlin", "generate3D");
	const auto computePipeline = Compute::Pipeline(*shader);

	const auto set =
		Memory::Descriptor::Set::Builder(Context::Scheduler().DescriptorPool(), computePipeline.DescriptorSetLayout(0))
			.WriteImages(0, inputImageInfos)
			.WriteImage(1,
				vk::DescriptorImageInfo()
					.setImageLayout(vk::ImageLayout::eGeneral)
					.setImageView(**outputImageView))
			.Build();

	Context::Device().RunSingleTimeCommand(
		[&](const Core::CommandBuffer& commandBuffer) {
			directionsNoise.Image().TransitionLayout(commandBuffer, vk::ImageLayout::eGeneral);
			computePipeline.Bind(commandBuffer);
			computePipeline.BindDescriptorSet(0, commandBuffer, *set);

			const struct PushConstants {
				u32 octaves;
			} pushConstants {
				.octaves = octaves
			};

			computePipeline.PushConstants(commandBuffer, vk::ShaderStageFlagBits::eCompute, 0, pushConstants);

			const auto groupSize = Math::Vector3u{8u, 8u, 8u};
			const Math::Vector3u dispatchSize = (size + (groupSize - 1u)) / groupSize;
			commandBuffer->dispatch(dispatchSize.x, dispatchSize.y, dispatchSize.z);

			m_image->TransitionLayout(commandBuffer, vk::ImageLayout::eShaderReadOnlyOptimal);
		},
		vk::QueueFlagBits::eCompute);
}
