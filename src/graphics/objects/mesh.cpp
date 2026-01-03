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

Coral::Graphics::Mesh::Builder::Builder(const UUID& uuid) {
	if (uuid == boost::uuids::nil_uuid()) {
		m_uuid = Coral::UUIDGenerator()();
	}
}
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
Coral::Graphics::Mesh::Builder&
Coral::Graphics::Mesh::Builder::VertexBuffer(std::unique_ptr<Memory::Buffer> vertexBuffer) {
	m_vertexBuffer = std::move(vertexBuffer);
	return *this;
}
Coral::Graphics::Mesh::Builder&
Coral::Graphics::Mesh::Builder::IndexBuffer(std::unique_ptr<Memory::Buffer> indexBuffer) {
	m_indexBuffer = std::move(indexBuffer);
	return *this;
}
std::unique_ptr<Coral::Graphics::Mesh> Coral::Graphics::Mesh::Builder::Build() { return std::make_unique<Mesh>(*this); }
Coral::Graphics::Mesh::Mesh(Builder& builder) {
	m_uuid = builder.m_uuid;
	m_name = builder.m_name;

	if (builder.m_vertexBuffer && builder.m_indexBuffer) {
		m_vertexBuffer = std::move(builder.m_vertexBuffer);
		m_indexBuffer = std::move(builder.m_indexBuffer);
	} else {
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
}
Coral::Graphics::Mesh::~Mesh() = default;
const Coral::UUID& Coral::Graphics::Mesh::Id() const { return m_uuid; }
const std::string& Coral::Graphics::Mesh::Name() const { return m_name; }
void Coral::Graphics::Mesh::Bind(const vk::CommandBuffer& commandBuffer) const {
	commandBuffer.bindVertexBuffers(0, { **m_vertexBuffer }, { 0 });
	commandBuffer.bindIndexBuffer(**m_indexBuffer, 0, vk::IndexType::eUint32);
}

void Coral::Graphics::Mesh::Draw(const vk::CommandBuffer& commandBuffer, const uint32_t instanceCount) const {
	commandBuffer.drawIndexed(m_indexBuffer->InstanceCount(), instanceCount, 0, 0, 0);
}

void Coral::Graphics::Mesh::CreateVertexBuffer(const std::vector<Vertex>& vertices) {
	m_vertexBuffer = Memory::Buffer::Builder()
		.InstanceSize(sizeof(Vertex))
		.InstanceCount(static_cast<uint32_t>(vertices.size()))
		.UsageFlags(vk::BufferUsageFlagBits::eVertexBuffer)
		.MemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal)
		.Data(vertices.data(), sizeof(Vertex) * vertices.size())
		.Build();
}

void Coral::Graphics::Mesh::CreateIndexBuffer(const std::vector<u32>& indices) {
	m_indexBuffer = Memory::Buffer::Builder()
		.InstanceSize(sizeof(u32))
		.InstanceCount(static_cast<u32>(indices.size()))
		.UsageFlags(vk::BufferUsageFlagBits::eIndexBuffer)
		.MemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal)
		.Data(indices.data(), sizeof(u32) * indices.size())
		.Build();
}
