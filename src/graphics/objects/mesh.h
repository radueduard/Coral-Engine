//
// Created by radue on 10/20/2024.
//

#pragma once

#include <memory>
#include <vector>

#include <boost/uuid/uuid.hpp>
#include <magic_enum/magic_enum.hpp>
#include <nlohmann/json.hpp>
#include <vulkan/vulkan.hpp>

#include "math/aabb.h"
#include "memory/buffer.h"

static vk::Format FormatFromString(const std::string& format) {
    if (format == "float" || format == "bool" || format == "int" || format == "uint") {
        return vk::Format::eR32Sfloat;
    }
    if (format == "vec2" || format == "bvec2" || format == "ivec2 " || format == "uvec2") {
        return vk::Format::eR32G32Sfloat;
    }
    if (format == "vec3" || format == "bvec3" || format == "ivec3" || format == "uvec3") {
        return vk::Format::eR32G32B32Sfloat;
    }
    if (format == "vec4" || format == "bvec4" || format == "ivec4" || format == "uvec4") {
        return vk::Format::eR32G32B32A32Sfloat;
    }
    if (format == "mat2" || format == "dmat2") {
        return vk::Format::eR32G32Sfloat;
    }
    if (format == "mat3" || format == "dmat3") {
        return vk::Format::eR32G32B32Sfloat;
    }
    if (format == "mat4" || format == "dmat4") {
        return vk::Format::eR32G32B32A32Sfloat;
    }
    throw std::runtime_error("Unknown format");
}

namespace Coral::Graphics {
    struct Vertex {
        enum class Attribute : u16 {
            Position = 1 << 0,
            Normal = 1 << 1,
            Tangent = 1 << 2,
            TexCoord0 = 1 << 3,
            TexCoord1 = 1 << 4,
            Color0 = 1 << 5
        };

        Math::Vector3<f32> position = {0.0f, 0.0f, 0.0f};
        Math::Vector3<f32> normal = {0.0f, 0.0f, 0.0f};
        Math::Vector4<f32> tangent = {0.0f, 0.0f, 0.0f, 1.0f};
        Math::Vector2<f32> texCoord0 = {0.0f, 0.0f};
        Math::Vector2<f32> texCoord1 = {0.0f, 0.0f};
        Math::Vector4<f32> color0 = {1.0f, 1.0f, 1.0f, 1.0f};

        static std::vector<vk::VertexInputBindingDescription> BindingDescriptions() {
            return {
                vk::VertexInputBindingDescription()
                    .setBinding(0)
                    .setStride(sizeof(Vertex))
                    .setInputRate(vk::VertexInputRate::eVertex)
            };
        }

        static std::vector<vk::VertexInputAttributeDescription> AttributeDescriptions(const nlohmann::json &inputAnalysis) {
            std::vector<vk::VertexInputAttributeDescription> attributeDescriptions;

            for (const auto &input : inputAnalysis) {
                const auto location = static_cast<u32>(input["location"].get<i32>());
                const auto format = FormatFromString(input["type"].get<String>());
            	auto attribute = magic_enum::enum_cast<Attribute>(input["attribute"].get<String>());
                const auto offset = Offset(attribute.value());


                attributeDescriptions.emplace_back(
                    vk::VertexInputAttributeDescription()
                        .setBinding(0)
                        .setLocation(location)
                        .setFormat(format)
                        .setOffset(static_cast<uint32_t>(offset)));
            }

            return attributeDescriptions;
        }
    private:
        static size_t Offset(const Attribute attribute) {
            switch (attribute) {
                case Attribute::Position: return offsetof(Vertex, position);
                case Attribute::Normal: return offsetof(Vertex, normal);
                case Attribute::Tangent: return offsetof(Vertex, tangent);
                case Attribute::TexCoord0: return offsetof(Vertex, texCoord0);
                case Attribute::TexCoord1: return offsetof(Vertex, texCoord1);
                case Attribute::Color0: return offsetof(Vertex, color0);
                default: throw std::runtime_error("Unknown attribute");
            }
        }
    };

    class Mesh {
    public:
        class Builder {
            friend class Mesh;
        public:
            explicit Builder(const boost::uuids::uuid &uuid) : m_uuid(uuid) {}
            ~Builder() = default;

            Builder& Name(const std::string &name) {
                m_name = name;
                return *this;
            }

            Builder& AddVertex(Vertex vertex) {
                m_vertices.emplace_back(vertex);
                return *this;
            }

            Builder& AddIndex(u32 index) {
                m_indices.emplace_back(index);
                return *this;
            }

        	Builder& AABB(const Math::AABB &aabb) {
				m_aabb = aabb;
				return *this;
			}

            std::unique_ptr<Mesh> Build() {
                return std::make_unique<Mesh>(*this);
            }
        private:
            UUID m_uuid;
            String m_name;
        	std::optional<Math::AABB> m_aabb = std::nullopt;
            std::vector<Vertex> m_vertices;
            std::vector<u32> m_indices;
        };

        explicit Mesh(Builder &builder) {
            m_uuid = builder.m_uuid;
            m_name = builder.m_name;

        	if (builder.m_aabb) {
        		m_aabb = builder.m_aabb.value();
        	} else {
        		m_aabb = Math::AABB(builder.m_vertices[0].position, builder.m_vertices[0].position);
				for (const auto &vertex : builder.m_vertices) {
					m_aabb.Grow(vertex.position);
				}
			}
            CreateVertexBuffer(builder.m_vertices);
            CreateIndexBuffer(builder.m_indices);
        }

        ~Mesh() = default;

        [[nodiscard]] const UUID &Id() const { return m_uuid; }
        [[nodiscard]] const std::string &Name() const { return m_name; }

        void Bind(const vk::CommandBuffer &commandBuffer) const {
            const vk::ArrayProxy<const vk::Buffer> buffers = { **m_vertexBuffer };
            const vk::ArrayProxy<const vk::DeviceSize> offsets = {0};
            commandBuffer.bindVertexBuffers(0, buffers, offsets);
            commandBuffer.bindIndexBuffer(**m_indexBuffer, 0, vk::IndexType::eUint32);
        }

        void Draw(const vk::CommandBuffer &commandBuffer, const uint32_t instanceCount = 1) const {
            commandBuffer.drawIndexed(m_indexBuffer->InstanceCount(), instanceCount, 0, 0, 0);
        }

    private:
        UUID m_uuid;
        String m_name;
    	Math::AABB m_aabb;
        std::unique_ptr<Memory::Buffer> m_indexBuffer;
        std::unique_ptr<Memory::Buffer> m_vertexBuffer;

        void CreateVertexBuffer(std::vector<Vertex> &vertices) {
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

        void CreateIndexBuffer(std::vector<u32> &indices) {
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
    };
}