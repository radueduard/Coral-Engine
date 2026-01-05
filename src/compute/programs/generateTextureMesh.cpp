//
// Created by radue on 1/3/2026.
//

#include "generateTextureMesh.h"

#include "compute/pipeline.h"
#include "core/scheduler.h"
#include "memory/buffer.h"
#include "shader/manager.h"

Coral::Compute::GenerateTextureMesh::GenerateTextureMesh (
	const Memory::Image& image,
	const Math::Vector3u& chunkCount
) : m_image(image), m_chunkCount(chunkCount) {}

std::unique_ptr<Coral::Graphics::Mesh> Coral::Compute::GenerateTextureMesh::Execute(
	const Math::Vector3u& offset,
	const Math::Vector3u& fullCount
) const {
	const u32 chunkCountTotal = m_chunkCount.x * m_chunkCount.y * m_chunkCount.z;

	const u32 initialCounts[2] = { 0, 0 };

	const auto counts = Memory::Buffer::Builder()
		.InstanceSize(sizeof(u32))
		.InstanceCount(2)
		.UsageFlags(vk::BufferUsageFlagBits::eStorageBuffer)
		.MemoryProperty(vk::MemoryPropertyFlagBits::eHostVisible)
		.MemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent)
		.Data(initialCounts, 2)
		.Build();

	// max vertex count is 12 * 8^3 * chunkCountTotal
	auto vertexBufferWithDuplicates = Memory::Buffer::Builder()
		.InstanceSize(sizeof(Graphics::Vertex))
		.InstanceCount(12 * 512 * chunkCountTotal)
		.UsageFlags(vk::BufferUsageFlagBits::eStorageBuffer)
		.UsageFlags(vk::BufferUsageFlagBits::eTransferSrc)
		.MemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal)
		.Build();

	// max index count is 15 * 8^3 * chunkCountTotal
	auto indexBufferWithDuplicates = Memory::Buffer::Builder()
		.InstanceSize(sizeof(u32))
		.InstanceCount(15 * 512 * chunkCountTotal)
		.UsageFlags(vk::BufferUsageFlagBits::eStorageBuffer)
		.UsageFlags(vk::BufferUsageFlagBits::eTransferSrc)
		.MemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal)
		.Build();

	const auto generateShader = Shader::Manager::Get().GetShader("texToMesh", "TextureToMesh3D");
	auto computePipeline = Compute::Pipeline(*generateShader);

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

	Context::Device().RunSingleTimeCommand([&](const Core::CommandBuffer& commandBuffer) {
		computePipeline.Bind(commandBuffer);
		computePipeline.BindDescriptorSet(0, commandBuffer, *generateSet);

		const struct  {
			alignas(16) Math::Vector3f gridMin;
			alignas(16) Math::Vector3f gridMax;
			alignas(16) Math::Vector3u chunkCount;
		} pushConstants = {
			.gridMin = Math::Vector3f { -25.f, -25.f, -25.f },
			.gridMax = Math::Vector3f { 25.f, 25.f, 25.f },
			.chunkCount = fullCount * m_chunkCount
		};

		computePipeline.PushConstants(commandBuffer, vk::ShaderStageFlagBits::eCompute, 0, pushConstants);
		commandBuffer->dispatchBase(
			offset.x * m_chunkCount.x, offset.y * m_chunkCount.y, offset.z * m_chunkCount.z,
			m_chunkCount.x, m_chunkCount.y, m_chunkCount.z
		);
	}, vk::QueueFlagBits::eCompute);

	auto mappedCounts = counts->Map<u32>();
	auto vertexCount = mappedCounts[0];
	auto indexCount = mappedCounts[1];
	counts->Unmap();

	if (vertexCount == 0 || indexCount == 0) {
		return nullptr;
	}

	std::cout << "Generated mesh with " << vertexCount << " vertices and " << indexCount << " indices." << std::endl;

	auto vertexBuffer = Memory::Buffer::Builder()
		.InstanceSize(sizeof(Graphics::Vertex))
		.InstanceCount(vertexCount)
		.UsageFlags(vk::BufferUsageFlagBits::eVertexBuffer)
		.UsageFlags(vk::BufferUsageFlagBits::eTransferDst)
		.MemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal)
		.Build();

	auto indexBuffer = Memory::Buffer::Builder()
		.InstanceSize(sizeof(u32))
		.InstanceCount(indexCount)
		.UsageFlags(vk::BufferUsageFlagBits::eIndexBuffer)
		.UsageFlags(vk::BufferUsageFlagBits::eTransferDst)
		.MemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal)
		.Build();

	vertexBuffer->CopyBuffer(*vertexBufferWithDuplicates, vertexCount);
	indexBuffer->CopyBuffer(*indexBufferWithDuplicates, indexCount);

	return Graphics::Mesh::Builder()
		.Name("GeneratedTextureMesh" + std::to_string(offset.x) + "_" + std::to_string(offset.y) + "_" + std::to_string(offset.z))
		.VertexBuffer(std::move(vertexBuffer))
		.IndexBuffer(std::move(indexBuffer))
		.AABB(Math::AABB { Math::Vector3f { -25.f, -25.f, -25.f }, Math::Vector3f { 25.f, 25.f, 25.f } })
		.Build();

	// return nullptr;
}


