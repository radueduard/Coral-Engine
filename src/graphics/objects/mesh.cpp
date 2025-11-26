//
// Created by radue on 11/24/2025.
//

#include "mesh.h"


#include "memory/buffer.h"
#include <magic_enum/magic_enum.hpp>

#include "shader/shader.h"


std::vector<vk::VertexInputBindingDescription> Coral::Graphics::Vertex::BindingDescriptions() {
	return {vk::VertexInputBindingDescription()
				.setBinding(0)
				.setStride(sizeof(Vertex))
				.setInputRate(vk::VertexInputRate::eVertex)};
}

static std::string AllCaps(std::string str) {
	std::ranges::transform(str, str.begin(), ::toupper);
	return str;
}

static std::string AllLower(std::string str) {
	std::ranges::transform(str, str.begin(), ::tolower);
	return str;
}

static std::string CapitalizeFirstLetter(const std::string& str) {
	std::string result = AllLower(str);
	if (!result.empty()) {
		result[0] = static_cast<char>(std::toupper(result[0]));
	}
	return result;
}

std::vector<vk::VertexInputAttributeDescription> Coral::Graphics::Vertex::AttributeDescriptions(const std::set<Shader::InOut>& inputAnalysis) {
	std::vector<vk::VertexInputAttributeDescription> attributeDescriptions;
	for (const auto& [location, name, format, semantic] : inputAnalysis) {
		auto attribute = magic_enum::enum_cast<Attribute>(AllCaps(semantic));
		const auto offset = Offset(attribute.value());

		attributeDescriptions.emplace_back(
			vk::VertexInputAttributeDescription().setBinding(0).setLocation(location).setFormat(format).setOffset(
				static_cast<uint32_t>(offset)));
	}

	return attributeDescriptions;
}

size_t Coral::Graphics::Vertex::Offset(const Attribute attribute) {
	switch (attribute) {
	case Attribute::POSITION:
		return offsetof(Vertex, position);
	case Attribute::NORMAL:
		return offsetof(Vertex, normal);
	case Attribute::TANGENT:
		return offsetof(Vertex, tangent);
	case Attribute::TEXCOORD:
		return offsetof(Vertex, texCoord0);
	case Attribute::TEXCOORD1:
		return offsetof(Vertex, texCoord1);
	case Attribute::COLOR:
		return offsetof(Vertex, color0);
	default:
		throw std::runtime_error("Unknown attribute");
	}
}

Coral::Graphics::Mesh::Builder::Builder(const boost::uuids::uuid& uuid) : m_uuid(uuid) {}
Coral::Graphics::Mesh::Builder::~Builder() = default;
Coral::Graphics::Mesh::Builder& Coral::Graphics::Mesh::Builder::Name(const std::string& name) {
	m_name = name;
	return *this;
}
Coral::Graphics::Mesh::Builder& Coral::Graphics::Mesh::Builder::AddVertex(Vertex vertex) {
	m_vertices.emplace_back(vertex);
	return *this;
}
Coral::Graphics::Mesh::Builder& Coral::Graphics::Mesh::Builder::AddIndex(u32 index) {
	m_indices.emplace_back(index);
	return *this;
}
Coral::Graphics::Mesh::Builder& Coral::Graphics::Mesh::Builder::AABB(const Math::AABB& aabb) {
	m_aabb = aabb;
	return *this;
}
std::unique_ptr<Coral::Graphics::Mesh> Coral::Graphics::Mesh::Builder::Build() { return std::make_unique<Mesh>(*this); }
Coral::Graphics::Mesh::Mesh(Builder& builder) {
	m_uuid = builder.m_uuid;
	m_name = builder.m_name;

	if (builder.m_aabb) {
		m_aabb = builder.m_aabb.value();
	}
	else {
		m_aabb = Math::AABB(builder.m_vertices[0].position, builder.m_vertices[0].position);
		for (const auto& vertex : builder.m_vertices) {
			m_aabb.Grow(vertex.position);
		}
	}
	CreateVertexBuffer(builder.m_vertices);
	CreateIndexBuffer(builder.m_indices);
}
Coral::Graphics::Mesh::~Mesh() = default;
const Coral::UUID& Coral::Graphics::Mesh::Id() const { return m_uuid; }
const std::string& Coral::Graphics::Mesh::Name() const { return m_name; }
void Coral::Graphics::Mesh::Bind(const vk::CommandBuffer& commandBuffer) const {
	const vk::ArrayProxy<const vk::Buffer> buffers = {**m_vertexBuffer};
	const vk::ArrayProxy<const vk::DeviceSize> offsets = {0};
	commandBuffer.bindVertexBuffers(0, buffers, offsets);
	commandBuffer.bindIndexBuffer(**m_indexBuffer, 0, vk::IndexType::eUint32);
}
void Coral::Graphics::Mesh::Draw(const vk::CommandBuffer& commandBuffer, const uint32_t instanceCount) const {
	commandBuffer.drawIndexed(m_indexBuffer->InstanceCount(), instanceCount, 0, 0, 0);
}
void Coral::Graphics::Mesh::CreateVertexBuffer(std::vector<Vertex>& vertices) {
	const auto stagingBuffer = Memory::Buffer::Builder()
								   .InstanceSize(sizeof(Vertex))
								   .InstanceCount(static_cast<uint32_t>(vertices.size()))
								   .UsageFlags(vk::BufferUsageFlagBits::eTransferSrc)
								   .MemoryProperty(vk::MemoryPropertyFlagBits::eHostVisible)
								   .MemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent)
								   .Build();

	stagingBuffer->Map<Vertex>();
	const auto copy = std::span(vertices.data(), vertices.size());
	stagingBuffer->Write(copy);
	stagingBuffer->Flush();
	stagingBuffer->Unmap();

	m_vertexBuffer = Memory::Buffer::Builder()
						 .InstanceSize(sizeof(Vertex))
						 .InstanceCount(static_cast<uint32_t>(vertices.size()))
						 .UsageFlags(vk::BufferUsageFlagBits::eTransferDst)
						 .UsageFlags(vk::BufferUsageFlagBits::eVertexBuffer)
						 .UsageFlags(vk::BufferUsageFlagBits::eStorageBuffer)
						 .MemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal)
						 .Build();

	m_vertexBuffer->CopyBuffer(stagingBuffer);
}
void Coral::Graphics::Mesh::CreateIndexBuffer(std::vector<u32>& indices) {
	const auto stagingBuffer = Memory::Buffer::Builder()
								   .InstanceSize(sizeof(u32))
								   .InstanceCount(static_cast<u32>(indices.size()))
								   .UsageFlags(vk::BufferUsageFlagBits::eTransferSrc)
								   .MemoryProperty(vk::MemoryPropertyFlagBits::eHostVisible)
								   .MemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent)
								   .Build();

	stagingBuffer->Map<u32>();
	const auto copy = std::span(indices.data(), indices.size());
	stagingBuffer->Write(copy);
	stagingBuffer->Flush();
	stagingBuffer->Unmap();

	m_indexBuffer = Memory::Buffer::Builder()
						.InstanceSize(sizeof(u32))
						.InstanceCount(static_cast<u32>(indices.size()))
						.UsageFlags(vk::BufferUsageFlagBits::eTransferDst)
						.UsageFlags(vk::BufferUsageFlagBits::eIndexBuffer)
						.UsageFlags(vk::BufferUsageFlagBits::eStorageBuffer)
						.MemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal)
						.Build();
	m_indexBuffer->CopyBuffer(stagingBuffer);
}
