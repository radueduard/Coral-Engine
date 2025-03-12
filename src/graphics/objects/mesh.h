//
// Created by radue on 10/20/2024.
//

#pragma once

#include <memory>
#include <vector>
#include <boost/uuid/uuid.hpp>
#include <glm/glm.hpp>
#include <unordered_set>
#include <unordered_map>
#include <variant>

#include <vulkan/vulkan.hpp>

#include "memory/buffer.h"

namespace mgv {
    struct Vertex {
        enum class Attribute : uint16_t {
            Position = 1 << 0,
            Normal = 1 << 1,
            Tangent = 1 << 2,
            TexCoord0 = 1 << 3,
            TexCoord1 = 1 << 4,
            Color0 = 1 << 5
        };

        glm::vec3 position;
        glm::vec3 normal;
        glm::vec4 tangent;
        glm::vec2 texCoord0;
        glm::vec2 texCoord1;
        glm::vec4 color0;

        static std::vector<vk::VertexInputBindingDescription> GetBindingDescriptions() {
            return {
                vk::VertexInputBindingDescription()
                    .setBinding(0)
                    .setStride(sizeof(Vertex))
                    .setInputRate(vk::VertexInputRate::eVertex)
            };
        }

        static std::vector<vk::VertexInputAttributeDescription> GetAttributeDescriptions(const std::unordered_set<Attribute> &attributes) {
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
            }
            return attributeDescriptions;
        }
    };

    class Mesh {
    public:
        class Builder {
            friend class Mesh;
        public:
            explicit Builder(const std::string &name) : m_name(name) {}
            ~Builder() = default;

            Builder& AddVertex(Vertex vertex) {
                m_vertices.emplace_back(vertex);
                return *this;
            }

            Builder& AddIndex(uint32_t index) {
                m_indices.emplace_back(index);
                return *this;
            }

            std::unique_ptr<Mesh> Build() {
                return std::make_unique<Mesh>(*this);
            }
        private:
            std::string m_name;
            std::vector<Vertex> m_vertices;
            std::vector<uint32_t> m_indices;
        };

        explicit Mesh(Builder &builder) {
            m_name = builder.m_name;
            CreateVertexBuffer(builder.m_vertices);
            CreateIndexBuffer(builder.m_indices);
        }

        ~Mesh() = default;

        void Bind(const vk::CommandBuffer &commandBuffer) const {
            const vk::ArrayProxy<const vk::Buffer> buffers = { **m_vertexBuffer };
            const vk::ArrayProxy<const vk::DeviceSize> offsets = {0};
            commandBuffer.bindVertexBuffers(0, buffers, offsets);
            commandBuffer.bindIndexBuffer(**m_indexBuffer, 0, vk::IndexType::eUint32);
        }

        void Draw(const vk::CommandBuffer &commandBuffer, const uint32_t instanceCount) const {
            commandBuffer.drawIndexed(m_indexBuffer->InstanceCount(), instanceCount, 0, 0, 0);
        }

    private:
        std::string m_name;
        std::unique_ptr<Memory::Buffer<uint32_t>> m_indexBuffer;
        std::unique_ptr<Memory::Buffer<Vertex>> m_vertexBuffer;

        void CreateVertexBuffer(std::vector<Vertex> &vertices) {
            const auto stagingBuffer = Memory::Buffer<Vertex>::Builder()
                .InstanceCount(static_cast<uint32_t>(vertices.size()))
                .UsageFlags(vk::BufferUsageFlagBits::eTransferSrc)
                .MemoryProperty(vk::MemoryPropertyFlagBits::eHostVisible)
                .MemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent)
                .Build();

            stagingBuffer->Map();
            const auto copy = std::span(vertices.data(), vertices.size());
            stagingBuffer->Write(copy);
            stagingBuffer->Flush();
            stagingBuffer->Unmap();

            m_vertexBuffer = Memory::Buffer<Vertex>::Builder()
                .InstanceCount(static_cast<uint32_t>(vertices.size()))
                .UsageFlags(vk::BufferUsageFlagBits::eTransferDst)
                .UsageFlags(vk::BufferUsageFlagBits::eVertexBuffer)
                .UsageFlags(vk::BufferUsageFlagBits::eStorageBuffer)
                .MemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal)
                .Build();

            m_vertexBuffer->CopyBuffer(stagingBuffer);
        }

        void CreateIndexBuffer(std::vector<uint32_t> &indices) {
            const auto stagingBuffer = Memory::Buffer<uint32_t>::Builder()
                .InstanceCount(static_cast<uint32_t>(indices.size()))
                .UsageFlags(vk::BufferUsageFlagBits::eTransferSrc)
                .MemoryProperty(vk::MemoryPropertyFlagBits::eHostVisible)
                .MemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent)
                .Build();

            stagingBuffer->Map();
            const auto copy = std::span(indices.data(), indices.size());
            stagingBuffer->Write(copy);
            stagingBuffer->Flush();
            stagingBuffer->Unmap();

            m_indexBuffer = Memory::Buffer<uint32_t>::Builder()
                .InstanceCount(static_cast<uint32_t>(indices.size()))
                .UsageFlags(vk::BufferUsageFlagBits::eTransferDst)
                .UsageFlags(vk::BufferUsageFlagBits::eIndexBuffer)
                .UsageFlags(vk::BufferUsageFlagBits::eStorageBuffer)
                .MemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal)
                .Build();
            m_indexBuffer->CopyBuffer(stagingBuffer);
        }
    };
}