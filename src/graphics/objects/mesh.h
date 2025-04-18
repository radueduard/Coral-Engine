//
// Created by radue on 10/20/2024.
//

#pragma once

#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include <nlohmann/json.hpp>

#include <vulkan/vulkan.hpp>

#include "memory/buffer.h"

#include <boost/uuid/uuid.hpp>


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

namespace Coral {
    struct Vertex {
        class Attribute {
        public:
            enum class Values : uint16_t {
                Position = 1 << 0,
                Normal = 1 << 1,
                Tangent = 1 << 2,
                TexCoord0 = 1 << 3,
                TexCoord1 = 1 << 4,
                Color0 = 1 << 5
            };

            Attribute(const Values value) : m_value(value) {}
            Attribute(const std::string& value) {
                if (value == "Position") m_value = Values::Position;
                else if (value == "Normal") m_value = Values::Normal;
                else if (value == "Tangent") m_value = Values::Tangent;
                else if (value == "TexCoord0") m_value = Values::TexCoord0;
                else if (value == "TexCoord1") m_value = Values::TexCoord1;
                else if (value == "Color0") m_value = Values::Color0;
                else throw std::runtime_error("Unknown attribute");
            }

            [[nodiscard]] bool operator==(const Attribute &other) const { return m_value == other.m_value; }
            [[nodiscard]] bool operator!=(const Attribute &other) const { return !(*this == other); }
            [[nodiscard]] bool operator<(const Attribute &other) const { return m_value < other.m_value; }
            uint16_t operator &(const Attribute &other) const { return static_cast<uint16_t>(m_value) & static_cast<uint16_t>(other.m_value); }

            static constexpr std::vector<Values> AllValues() {
                return {
                    Values::Position,
                    Values::Normal,
                    Values::Tangent,
                    Values::TexCoord0,
                    Values::TexCoord1,
                    Values::Color0
                };
            }

            template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
            operator T() const {
                return static_cast<T>(m_value);
            }

            operator Attribute::Values() const {
                return m_value;
            }

            operator std::string() const {
                switch (m_value) {
                    case Values::Position: return "Position";
                    case Values::Normal: return "Normal";
                    case Values::Tangent: return "Tangent";
                    case Values::TexCoord0: return "TexCoord0";
                    case Values::TexCoord1: return "TexCoord1";
                    case Values::Color0: return "Color0";
                    default: throw std::runtime_error("Unknown attribute");
                }
            }

        private:
            Values m_value;
        };

        glm::vec3 position;
        glm::vec3 normal;
        glm::vec4 tangent;
        glm::vec2 texCoord0;
        glm::vec2 texCoord1;
        glm::vec4 color0;

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
                const auto location = static_cast<uint32_t>(input["location"].get<int>());
                const auto format = FormatFromString(input["type"].get<std::string>());
                const auto offset = Offset(Attribute(input["attribute"].get<std::string>()));


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
            switch (static_cast<Attribute::Values>(attribute)) {
                case Attribute::Values::Position: return offsetof(Vertex, position);
                case Attribute::Values::Normal: return offsetof(Vertex, normal);
                case Attribute::Values::Tangent: return offsetof(Vertex, tangent);
                case Attribute::Values::TexCoord0: return offsetof(Vertex, texCoord0);
                case Attribute::Values::TexCoord1: return offsetof(Vertex, texCoord1);
                case Attribute::Values::Color0: return offsetof(Vertex, color0);
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

            Builder& AddIndex(uint32_t index) {
                m_indices.emplace_back(index);
                return *this;
            }

            std::unique_ptr<Mesh> Build() {
                return std::make_unique<Mesh>(*this);
            }
        private:
            boost::uuids::uuid m_uuid;
            std::string m_name;
            std::vector<Vertex> m_vertices;
            std::vector<uint32_t> m_indices;
        };

        explicit Mesh(Builder &builder) {
            m_uuid = builder.m_uuid;
            m_name = builder.m_name;
            CreateVertexBuffer(builder.m_vertices);
            CreateIndexBuffer(builder.m_indices);
        }

        ~Mesh() = default;

        [[nodiscard]] const boost::uuids::uuid &UUID() const { return m_uuid; }
        [[nodiscard]] const std::string &Name() const { return m_name; }

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
        boost::uuids::uuid m_uuid;
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

namespace std {
    template<>
    struct hash<Coral::Vertex::Attribute> {
        size_t operator()(const Coral::Vertex::Attribute &attribute) const noexcept {
            return hash<uint16_t>()(attribute);
        }
    };

    inline string to_string(const Coral::Vertex::Attribute::Values &attribute) {
        switch (attribute) {
            case Coral::Vertex::Attribute::Values::Position: return "Position";
            case Coral::Vertex::Attribute::Values::Normal: return "Normal";
            case Coral::Vertex::Attribute::Values::Tangent: return "Tangent";
            case Coral::Vertex::Attribute::Values::TexCoord0: return "TexCoord0";
            case Coral::Vertex::Attribute::Values::TexCoord1: return "TexCoord1";
            case Coral::Vertex::Attribute::Values::Color0: return "Color0";
            default: return "Unknown";
        }
    }
}