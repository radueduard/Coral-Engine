//
// Created by radue on 10/20/2024.
//

#include "mesh.h"

#include "memory/buffer.h"

namespace mgv {
    Mesh::Builder &Mesh::Builder::AddVertex(const Vertex &vertex) {
        m_vertices.emplace_back(vertex);
        return *this;
    }

    Mesh::Builder &Mesh::Builder::AddIndex(uint32_t index) {
        m_indices.emplace_back(index);
        return *this;
    }

    std::unique_ptr<Mesh> Mesh::Builder::Build(const Core::Device &device) const {
        return std::make_unique<Mesh>(device, *this);
    }

    Mesh::Mesh(const Core::Device& device, const Builder &builder)
        : m_device(device), m_name(builder.m_name)
    {
        CreateVertexBuffer(builder.m_vertices);
        CreateIndexBuffer(builder.m_indices);
    }

    Mesh::~Mesh() {
        m_vertexBuffer.reset();
        m_indexBuffer.reset();
    }


    void Mesh::CreateVertexBuffer(const std::vector<Vertex> &vertices) {
        const auto stagingBuffer = std::make_unique<Memory::Buffer>(
            m_device,
            sizeof(Vertex), static_cast<uint32_t>(vertices.size()),
            vk::BufferUsageFlagBits::eTransferSrc,
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

        stagingBuffer->Map<Vertex>();
        const auto copy = std::span(vertices.data(), vertices.size());
        stagingBuffer->Write(copy);
        stagingBuffer->Flush();
        stagingBuffer->Unmap();

        m_vertexBuffer = std::make_unique<Memory::Buffer>(
            m_device,
            sizeof(Vertex), static_cast<uint32_t>(vertices.size()),
            vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eStorageBuffer,
            vk::MemoryPropertyFlagBits::eDeviceLocal);
        m_vertexBuffer->CopyBuffer(stagingBuffer);
    }

    void Mesh::CreateIndexBuffer(const std::vector<uint32_t> &indices) {
        const auto stagingBuffer = std::make_unique<Memory::Buffer>(
            m_device,
            sizeof(uint32_t), static_cast<uint32_t>(indices.size()),
            vk::BufferUsageFlagBits::eTransferSrc,
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

        stagingBuffer->Map<uint32_t>();
        const auto copy = std::span(indices.data(), indices.size());
        stagingBuffer->Write(copy);
        stagingBuffer->Flush();
        stagingBuffer->Unmap();

        m_indexBuffer = std::make_unique<Memory::Buffer>(
            m_device,
            sizeof(uint32_t), static_cast<uint32_t>(indices.size()),
            vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eStorageBuffer,
            vk::MemoryPropertyFlagBits::eDeviceLocal);
        m_indexBuffer->CopyBuffer(stagingBuffer);
    }

    void Mesh::Bind(const vk::CommandBuffer &commandBuffer) const {
        const vk::ArrayProxy<const vk::Buffer> buffers = {**m_vertexBuffer};
        const vk::ArrayProxy<const vk::DeviceSize> offsets = {0};
        commandBuffer.bindVertexBuffers(0, buffers, offsets);
        commandBuffer.bindIndexBuffer(**m_indexBuffer, 0, vk::IndexType::eUint32);
    }

    void Mesh::Draw(const vk::CommandBuffer &commandBuffer, const uint32_t instanceCount) const {
        commandBuffer.drawIndexed(m_indexBuffer->InstanceCount(), instanceCount, 0, 0, 0);
    }

    std::vector<vk::VertexInputBindingDescription> Mesh::Vertex::GetBindingDescriptions() {
        return {
            vk::VertexInputBindingDescription()
                .setBinding(0)
                .setStride(sizeof(Vertex))
                .setInputRate(vk::VertexInputRate::eVertex)
        };
    }

    std::vector<vk::VertexInputAttributeDescription> Mesh::Vertex::GetAttributeDescriptions(const std::unordered_set<Attribute> &attributes) {
        std::vector<vk::VertexInputAttributeDescription> attributeDescriptions;

        uint32_t location = 0;
        if (attributes.contains(Attribute::Position)) {
            attributeDescriptions.emplace_back(
                vk::VertexInputAttributeDescription()
                    .setLocation(location++)
                    .setBinding(0)
                    .setFormat(vk::Format::eR32G32B32Sfloat)
                    .setOffset(offsetof(Vertex, position))
            );
        }
        if (attributes.contains(Attribute::Normal)) {
            attributeDescriptions.emplace_back(
                vk::VertexInputAttributeDescription()
                    .setLocation(location++)
                    .setBinding(0)
                    .setFormat(vk::Format::eR32G32B32Sfloat)
                    .setOffset(offsetof(Vertex, normal))
            );
        }
        if (attributes.contains(Attribute::Tangent)) {
            attributeDescriptions.emplace_back(
                vk::VertexInputAttributeDescription()
                    .setLocation(location++)
                    .setBinding(0)
                    .setFormat(vk::Format::eR32G32B32A32Sfloat)
                    .setOffset(offsetof(Vertex, tangent))
            );
        }
        if (attributes.contains(Attribute::TexCoord0)) {
            attributeDescriptions.emplace_back(
                vk::VertexInputAttributeDescription()
                    .setLocation(location++)
                    .setBinding(0)
                    .setFormat(vk::Format::eR32G32Sfloat)
                    .setOffset(offsetof(Vertex, texCoord0))
            );
        }
        if (attributes.contains(Attribute::TexCoord1)) {
            attributeDescriptions.emplace_back(
                vk::VertexInputAttributeDescription()
                    .setLocation(location++)
                    .setBinding(0)
                    .setFormat(vk::Format::eR32G32Sfloat)
                    .setOffset(offsetof(Vertex, texCoord1))
            );
        }
        if (attributes.contains(Attribute::Color0)) {
            attributeDescriptions.emplace_back(
                vk::VertexInputAttributeDescription()
                    .setLocation(location++)
                    .setBinding(0)
                    .setFormat(vk::Format::eR32G32B32A32Sfloat)
                    .setOffset(offsetof(Vertex, color0))
            );
        }
        return attributeDescriptions;
    }
}


