//
// Created by radue on 10/20/2024.
//

#include "mesh.h"

Mesh::Builder &Mesh::Builder::Type(Mesh::Type type) {
    m_type = type;
    return *this;
}

Mesh::Builder &Mesh::Builder::AddVertex(const Vertex &vertex) {
    switch (m_type) {
        case StructureOfArrays:
            m_positions.emplace_back(vertex.position);
            m_normals.emplace_back(vertex.normal);
            m_tangents.emplace_back(vertex.tangent);
            m_texCoords0.emplace_back(vertex.texCoord0);
            m_texCoords1.emplace_back(vertex.texCoord1);
            m_colors0.emplace_back(vertex.color0);
            break;
        case ArrayOfStructures:
            m_vertices.emplace_back(vertex);
            break;
    }
    return *this;
}

Mesh::Builder &Mesh::Builder::AddIndex(uint32_t index) {
    m_indices.emplace_back(index);
    return *this;
}

std::shared_ptr<Mesh> Mesh::Builder::Build() {
    return std::make_shared<Mesh>(*this);
}

Mesh::Mesh(const Builder &builder) : m_device(builder.m_device), m_type(builder.m_type) {
    CreateIndexBuffer(builder.m_indices);
    switch (m_type) {
        case StructureOfArrays:
            m_positionBuffer = CreateVertexBuffer(builder.m_positions);
            m_normalBuffer = CreateVertexBuffer(builder.m_normals);
            m_tangentBuffer = CreateVertexBuffer(builder.m_tangents);
            m_texCoord0Buffer = CreateVertexBuffer(builder.m_texCoords0);
            m_texCoord1Buffer = CreateVertexBuffer(builder.m_texCoords1);
            m_color0Buffer = CreateVertexBuffer(builder.m_colors0);
            break;
        case ArrayOfStructures:
            m_vertexBuffer = CreateVertexBuffer(builder.m_vertices);
            break;
    }
}

Mesh::~Mesh() {
    switch (m_type) {
        case StructureOfArrays:
            m_color0Buffer.reset();
            m_texCoord1Buffer.reset();
            m_texCoord0Buffer.reset();
            m_tangentBuffer.reset();
            m_normalBuffer.reset();
            m_positionBuffer.reset();
            break;
        case ArrayOfStructures:
            m_vertexBuffer.reset();
            break;
    }
    m_indexBuffer.reset();
}


void Mesh::CreateIndexBuffer(const std::vector<uint32_t> &indices) {
    auto stagingBuffer = Memory::Buffer<uint32_t>(
            m_device,
            static_cast<uint32_t>(indices.size()),
            vk::BufferUsageFlagBits::eTransferSrc,
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

    stagingBuffer.Map();
    stagingBuffer.Write(indices.data());
    stagingBuffer.Flush();
    stagingBuffer.Unmap();

    m_indexBuffer = std::make_unique<Memory::Buffer<uint32_t>>(
        m_device,
        static_cast<uint32_t>(indices.size()),
        vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eStorageBuffer,
        vk::MemoryPropertyFlagBits::eDeviceLocal);
    m_indexBuffer->CopyBuffer(stagingBuffer);
}

void Mesh::Bind(const vk::CommandBuffer &commandBuffer) const {
    vk::ArrayProxy<const vk::Buffer> buffers;
    vk::ArrayProxy<const vk::DeviceSize> offsets;
    switch (m_type) {
        case StructureOfArrays:
            buffers = {
                **m_positionBuffer,
                **m_normalBuffer,
                **m_tangentBuffer,
                **m_texCoord0Buffer,
                **m_texCoord1Buffer,
                **m_color0Buffer
            };
            offsets = {0, 0, 0, 0, 0, 0};
            break;
        case ArrayOfStructures:
            buffers = {**m_vertexBuffer};
            offsets = {0};
            break;
    }
    commandBuffer.bindVertexBuffers(0, buffers, offsets);
    commandBuffer.bindIndexBuffer(**m_indexBuffer, 0, vk::IndexType::eUint32);
}

std::vector<vk::VertexInputBindingDescription> Mesh::Vertex::GetBindingDescriptions(const Type type) {
    std::vector<vk::VertexInputBindingDescription> bindingDescriptions;
    switch (type) {
        case StructureOfArrays:
            bindingDescriptions.emplace_back(
                vk::VertexInputBindingDescription()
                    .setBinding(0)
                    .setStride(sizeof(glm::vec3))
                    .setInputRate(vk::VertexInputRate::eVertex)
            );
            bindingDescriptions.emplace_back(
                vk::VertexInputBindingDescription()
                    .setBinding(1)
                    .setStride(sizeof(glm::vec3))
                    .setInputRate(vk::VertexInputRate::eVertex)
            );
            bindingDescriptions.emplace_back(
                vk::VertexInputBindingDescription()
                    .setBinding(2)
                    .setStride(sizeof(glm::vec4))
                    .setInputRate(vk::VertexInputRate::eVertex)
            );
            bindingDescriptions.emplace_back(
                vk::VertexInputBindingDescription()
                    .setBinding(3)
                    .setStride(sizeof(glm::vec2))
                    .setInputRate(vk::VertexInputRate::eVertex)
            );
            bindingDescriptions.emplace_back(
                vk::VertexInputBindingDescription()
                    .setBinding(4)
                    .setStride(sizeof(glm::vec2))
                    .setInputRate(vk::VertexInputRate::eVertex)
            );
            bindingDescriptions.emplace_back(
                vk::VertexInputBindingDescription()
                    .setBinding(5)
                    .setStride(sizeof(glm::vec4))
                    .setInputRate(vk::VertexInputRate::eVertex)
            );
            break;
        case ArrayOfStructures:
            bindingDescriptions.emplace_back(
                vk::VertexInputBindingDescription()
                    .setBinding(0)
                    .setStride(sizeof(Vertex))
                    .setInputRate(vk::VertexInputRate::eVertex)
            );
            break;
    }
    return bindingDescriptions;
}

std::vector<vk::VertexInputAttributeDescription> Mesh::Vertex::GetAttributeDescriptions(const Type type, const vk::Flags<Attribute> attributes) {
    std::vector<vk::VertexInputAttributeDescription> attributeDescriptions;

    uint32_t location = 0;
    if (attributes & Attribute::Position) {
        attributeDescriptions.emplace_back(
            vk::VertexInputAttributeDescription()
                .setLocation(location++)
                .setBinding(0)
                .setFormat(vk::Format::eR32G32B32Sfloat)
                .setOffset(type == Type::StructureOfArrays ? 0 : offsetof(Vertex, position))
        );
    }
    if (attributes & Attribute::Normal) {
        attributeDescriptions.emplace_back(
            vk::VertexInputAttributeDescription()
                .setLocation(location++)
                .setBinding(type == Type::StructureOfArrays ? 1 : 0)
                .setFormat(vk::Format::eR32G32B32Sfloat)
                .setOffset(type == Type::StructureOfArrays ? 0 : offsetof(Vertex, normal))
        );
    }
    if (attributes & Attribute::Tangent) {
        attributeDescriptions.emplace_back(
            vk::VertexInputAttributeDescription()
                .setLocation(location++)
                .setBinding(type == Type::StructureOfArrays ? 2 : 0)
                .setFormat(vk::Format::eR32G32B32A32Sfloat)
                .setOffset(type == Type::StructureOfArrays ? 0 : offsetof(Vertex, tangent))
        );
    }
    if (attributes & Attribute::TexCoord0) {
        attributeDescriptions.emplace_back(
            vk::VertexInputAttributeDescription()
                .setLocation(location++)
                .setBinding(type == Type::StructureOfArrays ? 3 : 0)
                .setFormat(vk::Format::eR32G32Sfloat)
                .setOffset(type == Type::StructureOfArrays ? 0 : offsetof(Vertex, texCoord0))
        );
    }
    if (attributes & Attribute::TexCoord1) {
        attributeDescriptions.emplace_back(
            vk::VertexInputAttributeDescription()
                .setLocation(location++)
                .setBinding(type == Type::StructureOfArrays ? 4 : 0)
                .setFormat(vk::Format::eR32G32Sfloat)
                .setOffset(type == Type::StructureOfArrays ? 0 : offsetof(Vertex, texCoord1))
        );
    }
    if (attributes & Attribute::Color0) {
        attributeDescriptions.emplace_back(
            vk::VertexInputAttributeDescription()
                .setLocation(location++)
                .setBinding(type == Type::StructureOfArrays ? 5 : 0)
                .setFormat(vk::Format::eR32G32B32A32Sfloat)
                .setOffset(type == Type::StructureOfArrays ? 0 : offsetof(Vertex, color0))
        );
    }
    return attributeDescriptions;
}


