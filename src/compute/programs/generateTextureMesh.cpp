//
// Created by radue on 1/3/2026.
//

#include "generateTextureMesh.h"

#include <fstream>

#include "compute/pipeline.h"
#include "core/scheduler.h"
#include "memory/buffer.h"
#include "shader/manager.h"

Coral::Compute::GenerateTextureMesh::GenerateTextureMesh (
	const Memory::Image& image,
	const Math::Vector3u& groupCount
) : m_image(image), m_groupCount(groupCount) {}

std::unique_ptr<Coral::Graphics::Mesh> Coral::Compute::GenerateTextureMesh::Execute(
	const Math::Vector3u& chunkID,
	const Math::Vector3u& chunkCount
) const {
	const u32 initialCounts[2] = { 0, 0 };

	const auto counts = Memory::Buffer::Builder()
		.InstanceSize(sizeof(u32))
		.InstanceCount(2)
		.UsageFlags(vk::BufferUsageFlagBits::eStorageBuffer)
		.MemoryProperty(vk::MemoryPropertyFlagBits::eHostVisible)
		.MemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent)
		.Data(initialCounts, 2)
		.Build();

	auto defaultVertex = Graphics::Vertex {
		.position = { std::numeric_limits<f32>::max(), std::numeric_limits<f32>::max(), std::numeric_limits<f32>::max() }
	};

	u32 indicesPerThread = 15;
	u32 verticesPerThread = 16;
	u32 threadsPerGroup = 8 * 8 * 8;
	u32 groupsPerChunk = m_groupCount.x * m_groupCount.y * m_groupCount.z;

	auto vertexBufferWithDuplicates = Memory::Buffer::Builder()
		.InstanceSize(sizeof(Graphics::Vertex))
		.InstanceCount(verticesPerThread * threadsPerGroup * groupsPerChunk)
		.UsageFlags(vk::BufferUsageFlagBits::eStorageBuffer)
		.UsageFlags(vk::BufferUsageFlagBits::eTransferSrc)
		.MemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal)
		.Data(&defaultVertex)
		.Build();

	std::vector<u32> initialIndices = std::ranges::iota_view(0u, indicesPerThread * threadsPerGroup * groupsPerChunk)
		| std::ranges::to<std::vector<u32>>();

	auto reverseRemapTableBuffer = Memory::Buffer::Builder()
		.InstanceSize(sizeof(u32))
		.InstanceCount(verticesPerThread * threadsPerGroup * groupsPerChunk)
		.UsageFlags(vk::BufferUsageFlagBits::eStorageBuffer)
		.UsageFlags(vk::BufferUsageFlagBits::eTransferSrc)
		.MemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal)
		.Data(initialIndices.data(), static_cast<u32>(initialIndices.size()))
		.Build();

	auto remapTableBuffer = Memory::Buffer::Builder()
		.InstanceSize(sizeof(u32))
		.InstanceCount(verticesPerThread * threadsPerGroup * groupsPerChunk)
		.UsageFlags(vk::BufferUsageFlagBits::eStorageBuffer)
		.UsageFlags(vk::BufferUsageFlagBits::eTransferSrc)
		.MemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal)
		.Data(initialIndices.data(), static_cast<u32>(initialIndices.size()))
		.Build();

	auto remapTableBuffer2 = Memory::Buffer::Builder()
		.InstanceSize(sizeof(u32))
		.InstanceCount(verticesPerThread * threadsPerGroup * groupsPerChunk)
		.UsageFlags(vk::BufferUsageFlagBits::eStorageBuffer)
		.UsageFlags(vk::BufferUsageFlagBits::eTransferSrc)
		.MemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal)
		.Data(initialIndices.data(), static_cast<u32>(initialIndices.size()))
		.Build();

	// max index count is 15 * 8^3 * chunkCountTotal
	auto indexBufferWithDuplicates = Memory::Buffer::Builder()
		.InstanceSize(sizeof(u32))
		.InstanceCount(indicesPerThread * threadsPerGroup * groupsPerChunk)
		.UsageFlags(vk::BufferUsageFlagBits::eStorageBuffer)
		.UsageFlags(vk::BufferUsageFlagBits::eTransferSrc)
		.MemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal)
		.Build();

	const auto generateShader = Context::ShaderManager().SlangShader("texToMesh", "TextureToMesh3D");
	auto computePipeline = Compute::Pipeline(*generateShader);

	const auto sortShader = Context::ShaderManager().SlangShader("vertex", "sort");
	auto sortPipeline = Compute::Pipeline(*sortShader);

	const auto reverseRemapShader = Context::ShaderManager().SlangShader("vertex", "reverseRemapTable");
	auto reverseRemapPipeline = Compute::Pipeline(*reverseRemapShader);

	const auto actualizeIndicesShader = Context::ShaderManager().SlangShader("vertex", "actualizeIndices");
	auto actualizeIndicesPipeline = Compute::Pipeline(*actualizeIndicesShader);

	const auto combineDuplicateShader = Context::ShaderManager().SlangShader("vertex", "combineDuplicates");
	auto combineDuplicatePipeline = Compute::Pipeline(*combineDuplicateShader);

	auto imageView = Memory::ImageView::Builder(m_image)
		.ViewType(vk::ImageViewType::e3D)
		.Build();

	auto sampler = Memory::Sampler::Builder()
		.AddressMode(vk::SamplerAddressMode::eClampToEdge)
		.Build();

	auto imageDescriptorInfo = vk::DescriptorImageInfo()
		.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
		.setImageView(**imageView)
		.setSampler(**sampler);

	auto generateSet = Memory::Descriptor::Set::Builder(Context::Scheduler().DescriptorPool(), computePipeline.DescriptorSetLayout(0))
		.WriteImage(0, imageDescriptorInfo)
		.WriteBuffer(1, vertexBufferWithDuplicates->DescriptorInfo())
		.WriteBuffer(2, indexBufferWithDuplicates->DescriptorInfo())
		.WriteBuffer(3, counts->DescriptorInfo())
		.Build();

	auto sortSet = Memory::Descriptor::Set::Builder(Context::Scheduler().DescriptorPool(), sortPipeline.DescriptorSetLayout(0))
		.WriteBuffer(0, vertexBufferWithDuplicates->DescriptorInfo())
		.WriteBuffer(1, reverseRemapTableBuffer->DescriptorInfo())
		.Build();

	auto reverseRemapSet = Memory::Descriptor::Set::Builder(Context::Scheduler().DescriptorPool(), reverseRemapPipeline.DescriptorSetLayout(0))
		.WriteBuffer(0, reverseRemapTableBuffer->DescriptorInfo())
		.WriteBuffer(1, remapTableBuffer->DescriptorInfo())
		.Build();

	auto actualizeIndicesSet = Memory::Descriptor::Set::Builder(Context::Scheduler().DescriptorPool(), actualizeIndicesPipeline.DescriptorSetLayout(0))
		.WriteBuffer(0, indexBufferWithDuplicates->DescriptorInfo())
		.WriteBuffer(1, remapTableBuffer->DescriptorInfo())
		.Build();

	auto combineDuplicateSet = Memory::Descriptor::Set::Builder(Context::Scheduler().DescriptorPool(), combineDuplicatePipeline.DescriptorSetLayout(0))
		.WriteBuffer(0, vertexBufferWithDuplicates->DescriptorInfo())
		.WriteBuffer(1, remapTableBuffer2->DescriptorInfo())
		.Build();

	auto actualizeIndicesSet2 = Memory::Descriptor::Set::Builder(Context::Scheduler().DescriptorPool(), actualizeIndicesPipeline.DescriptorSetLayout(0))
		.WriteBuffer(0, indexBufferWithDuplicates->DescriptorInfo())
		.WriteBuffer(1, remapTableBuffer2->DescriptorInfo())
		.Build();

	Context::Device().RunSingleTimeCommand([&](const Core::CommandBuffer& commandBuffer) {
		computePipeline.Bind(commandBuffer);
		computePipeline.BindDescriptorSet(0, commandBuffer, *generateSet);

		const struct {
			alignas(16) Math::Vector3f gridMin;
			alignas(16) Math::Vector3f gridMax;
			alignas(16) Math::Vector3u chunkID;
			alignas(16) Math::Vector3u chunkCount;
		} pushConstants = {
			.gridMin = Math::Vector3f { -25.f, -25.f, -25.f },
			.gridMax = Math::Vector3f { 25.f, 25.f, 25.f },
			.chunkID = chunkID,
			.chunkCount = chunkCount
		};

		computePipeline.PushConstants(commandBuffer, vk::ShaderStageFlagBits::eCompute, 0, pushConstants);
		commandBuffer->dispatch(m_groupCount.x, m_groupCount.y, m_groupCount.z);
	}, vk::QueueFlagBits::eCompute);

	auto mappedCounts = counts->Map<u32>();
	const auto vertexCount = mappedCounts[0];
	const auto indexCount = mappedCounts[1];
	counts->Unmap();

	if (vertexCount == 0 || indexCount == 0) {
		return nullptr;
	}

	std::cout << "Generated mesh with " << vertexCount << " vertices and " << indexCount << " indices." << std::endl;

	Context::Device().RunSingleTimeCommand([&](const Core::CommandBuffer& commandBuffer) {
		sortPipeline.Bind(commandBuffer);
		sortPipeline.BindDescriptorSet(0, commandBuffer, *sortSet);

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

		const u32 n = vertexBufferWithDuplicates->InstanceCount();
		constexpr u32 workGroupSize = 64;
		const u32 workGroupCount = n / (workGroupSize * 2);

		auto dispatch = [&](const u32 h, const Algorithm& algorithm) {
			const PushConstants pushConstantsSort {
				.algorithm = algorithm,
				.h = h
			};
			sortPipeline.PushConstants(commandBuffer, vk::ShaderStageFlagBits::eCompute, 0, pushConstantsSort);
			commandBuffer->dispatch(workGroupCount, 1, 1);

			auto vertexBarrier = vk::BufferMemoryBarrier()
				.setSrcAccessMask(vk::AccessFlagBits::eShaderWrite | vk::AccessFlagBits::eShaderRead)
				.setDstAccessMask(vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eShaderWrite)
				.setBuffer(**vertexBufferWithDuplicates)
				.setSize(vk::WholeSize);

			auto reverseRemapBarrier = vk::BufferMemoryBarrier()
				.setSrcAccessMask(vk::AccessFlagBits::eShaderWrite | vk::AccessFlagBits::eShaderRead)
				.setDstAccessMask(vk::AccessFlagBits::eShaderWrite | vk::AccessFlagBits::eShaderRead)
				.setBuffer(**reverseRemapTableBuffer)
				.setSize(vk::WholeSize);

			commandBuffer->pipelineBarrier(
				vk::PipelineStageFlagBits::eComputeShader,
				vk::PipelineStageFlagBits::eComputeShader,
				{},
				{},
				{ vertexBarrier, reverseRemapBarrier },
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

		auto reverseRemapBarrier = vk::BufferMemoryBarrier()
			.setSrcAccessMask(vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eShaderWrite)
			.setDstAccessMask(vk::AccessFlagBits::eShaderRead)
			.setBuffer(**reverseRemapTableBuffer)
			.setSize(vk::WholeSize);

		commandBuffer->pipelineBarrier(
			vk::PipelineStageFlagBits::eComputeShader,
			vk::PipelineStageFlagBits::eComputeShader,
			{},
			{},
			{ reverseRemapBarrier },
			{}
		);

		reverseRemapPipeline.Bind(commandBuffer);
		reverseRemapPipeline.BindDescriptorSet(0, commandBuffer, *reverseRemapSet);
		commandBuffer->dispatch(indexCount / 128 + 1, 1, 1);

		auto indexBarrier = vk::BufferMemoryBarrier()
			.setSrcAccessMask(vk::AccessFlagBits::eShaderWrite)
			.setDstAccessMask(vk::AccessFlagBits::eShaderWrite)
			.setBuffer(**indexBufferWithDuplicates)
			.setSize(vk::WholeSize);

		auto remapTableBarrier = vk::BufferMemoryBarrier()
			.setSrcAccessMask(vk::AccessFlagBits::eShaderWrite)
			.setDstAccessMask(vk::AccessFlagBits::eShaderRead)
			.setBuffer(**remapTableBuffer)
			.setSize(vk::WholeSize);

		commandBuffer->pipelineBarrier(
			vk::PipelineStageFlagBits::eComputeShader,
			vk::PipelineStageFlagBits::eComputeShader,
			{},
			{},
			{ indexBarrier, remapTableBarrier },
			{}
		);

		actualizeIndicesPipeline.Bind(commandBuffer);
		actualizeIndicesPipeline.BindDescriptorSet(0, commandBuffer, *actualizeIndicesSet);
		commandBuffer->dispatch(indexCount / 128 + 1, 1, 1);

		auto vertexBarrier2 = vk::BufferMemoryBarrier()
			.setSrcAccessMask(vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eShaderWrite)
			.setDstAccessMask(vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eShaderWrite)
			.setBuffer(**vertexBufferWithDuplicates)
			.setSize(vk::WholeSize);

		commandBuffer->pipelineBarrier(
			vk::PipelineStageFlagBits::eComputeShader,
			vk::PipelineStageFlagBits::eComputeShader,
			{},
			{},
			{ vertexBarrier2 },
			{}
		);

		combineDuplicatePipeline.Bind(commandBuffer);
		combineDuplicatePipeline.BindDescriptorSet(0, commandBuffer, *combineDuplicateSet);
		commandBuffer->dispatch(vertexCount / 128 + 1, 1, 1);

		auto remapTableBarrier2 = vk::BufferMemoryBarrier()
			.setSrcAccessMask(vk::AccessFlagBits::eShaderWrite)
			.setDstAccessMask(vk::AccessFlagBits::eShaderRead)
			.setBuffer(**remapTableBuffer2)
			.setSize(vk::WholeSize);

		indexBarrier = vk::BufferMemoryBarrier()
			.setSrcAccessMask(vk::AccessFlagBits::eShaderWrite)
			.setDstAccessMask(vk::AccessFlagBits::eShaderWrite)
			.setBuffer(**indexBufferWithDuplicates)
			.setSize(vk::WholeSize);

		commandBuffer->pipelineBarrier(
			vk::PipelineStageFlagBits::eComputeShader,
			vk::PipelineStageFlagBits::eComputeShader,
			{},
			{},
			{ remapTableBarrier2, indexBarrier },
			{}
		);

		actualizeIndicesPipeline.Bind(commandBuffer);
		actualizeIndicesPipeline.BindDescriptorSet(0, commandBuffer, *actualizeIndicesSet2);
		commandBuffer->dispatch(indexCount / 128 + 1, 1, 1);
	}, vk::QueueFlagBits::eCompute);

	auto vertexBuffer = Memory::Buffer::Builder()
		.InstanceSize(sizeof(Graphics::Vertex))
		.InstanceCount(vertexCount)
		.UsageFlags(vk::BufferUsageFlagBits::eVertexBuffer)
		.UsageFlags(vk::BufferUsageFlagBits::eStorageBuffer)
		.UsageFlags(vk::BufferUsageFlagBits::eTransferDst)
		.MemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal)
		.Build();

	auto indexBuffer = Memory::Buffer::Builder()
		.InstanceSize(sizeof(u32))
		.InstanceCount(indexCount)
		.UsageFlags(vk::BufferUsageFlagBits::eIndexBuffer)
		.UsageFlags(vk::BufferUsageFlagBits::eStorageBuffer)
		.UsageFlags(vk::BufferUsageFlagBits::eTransferDst)
		.MemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal)
		.Build();

	vertexBuffer->CopyBuffer(*vertexBufferWithDuplicates, vertexCount);
	indexBuffer->CopyBuffer(*indexBufferWithDuplicates, indexCount);

	return Graphics::Mesh::Builder()
		.Name("GeneratedTextureMesh" + std::to_string(chunkID.x) + "_" + std::to_string(chunkID.y) + "_" + std::to_string(chunkID.z))
		.VertexBuffer(std::move(vertexBuffer))
		.IndexBuffer(std::move(indexBuffer))
		.AABB(Math::AABB { Math::Vector3f { -25.f, -25.f, -25.f }, Math::Vector3f { 25.f, 25.f, 25.f } })
		.Build();

	// return nullptr;
}


