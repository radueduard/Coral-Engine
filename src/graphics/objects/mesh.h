//
// Created by radue on 10/20/2024.
//

#pragma once

#include <memory>
#include <vector>
#include <glm/glm.hpp>

#include "core/device.h"
#include "memory/buffer.h"

class Mesh {
public:
    enum Type {
        StructureOfArrays = 0,
        ArrayOfStructures = 1
    };

    enum Attribute {
        Position = 1 << 0,
        Normal = 1 << 1,
        Tangent = 1 << 2,
        TexCoord0 = 1 << 3,
        TexCoord1 = 1 << 4,
        Color0 = 1 << 5
    };
    struct Vertex {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec4 tangent;
        glm::vec2 texCoord0;
        glm::vec2 texCoord1;
        glm::vec4 color0;

        static std::vector<vk::VertexInputBindingDescription> GetBindingDescriptions(Type type);
        static std::vector<vk::VertexInputAttributeDescription> GetAttributeDescriptions(Type type, vk::Flags<Attribute> attributes);
    };

    class Builder {
        friend class Mesh;
    public:
        explicit Builder(const Core::Device &device) : m_device{device} {}

        Builder& Type(Mesh::Type type);
        Builder& AddIndex(uint32_t index);
        Builder& AddVertex(const Vertex &vertex);

        std::shared_ptr<Mesh> Build();
    private:
        const Core::Device &m_device;
        Mesh::Type m_type = ArrayOfStructures;

        std::vector<uint32_t> m_indices;
        std::vector<Vertex> m_vertices;

        std::vector<glm::vec3> m_positions;
        std::vector<glm::vec3> m_normals;
        std::vector<glm::vec4> m_tangents;
        std::vector<glm::vec2> m_texCoords0;
        std::vector<glm::vec2> m_texCoords1;
        std::vector<glm::vec4> m_colors0;
    };

    explicit Mesh(const Builder &builder);
    ~Mesh();

    Mesh(const Mesh &) = delete;
    Mesh &operator=(const Mesh &) = delete;

    void Bind(const vk::CommandBuffer &commandBuffer) const;


private:

    const Core::Device &m_device;
    Type m_type;
    std::unique_ptr<Memory::Buffer<uint32_t>> m_indexBuffer;

    std::unique_ptr<Memory::Buffer<Vertex>> m_vertexBuffer;
    std::unique_ptr<Memory::Buffer<glm::vec3>> m_positionBuffer;
    std::unique_ptr<Memory::Buffer<glm::vec3>> m_normalBuffer;
    std::unique_ptr<Memory::Buffer<glm::vec4>> m_tangentBuffer;
    std::unique_ptr<Memory::Buffer<glm::vec2>> m_texCoord0Buffer;
    std::unique_ptr<Memory::Buffer<glm::vec2>> m_texCoord1Buffer;
    std::unique_ptr<Memory::Buffer<glm::vec4>> m_color0Buffer;

    template<typename T>
    std::unique_ptr<Memory::Buffer<T>> CreateVertexBuffer(const std::vector<T> &vertices);
    void CreateIndexBuffer(const std::vector<uint32_t> &indices);
};

template <typename T>
std::unique_ptr<Memory::Buffer<T>> Mesh::CreateVertexBuffer(const std::vector<T> &vertices) {
    auto stagingBuffer = Memory::Buffer<T>(
            m_device,
            static_cast<uint32_t>(vertices.size()),
            vk::BufferUsageFlagBits::eTransferSrc,
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

    stagingBuffer.Map();
    stagingBuffer.Write(vertices.data());
    stagingBuffer.Flush();
    stagingBuffer.Unmap();

    auto vertexBuffer = std::make_unique<Memory::Buffer<T>>(
        m_device,
        static_cast<uint32_t>(vertices.size()),
        vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eStorageBuffer,
        vk::MemoryPropertyFlagBits::eDeviceLocal);
    vertexBuffer->CopyBuffer(stagingBuffer);
    return vertexBuffer;
}