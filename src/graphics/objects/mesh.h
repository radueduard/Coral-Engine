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

#include <vulkan/vulkan.hpp>

#include "memory/buffer.h"

namespace mgv {
    class Mesh {
    public:
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

            static std::vector<vk::VertexInputBindingDescription> GetBindingDescriptions();
            static std::vector<vk::VertexInputAttributeDescription> GetAttributeDescriptions(const std::unordered_set<Attribute> &attributes);
        };

        class Builder {
            friend class Mesh;
        public:
            explicit Builder(std::string name) : m_name(std::move(name)) {}

            Builder& AddIndex(uint32_t index);
            Builder& AddVertex(const Vertex &vertex);

            std::unique_ptr<Mesh> Build();
        private:
            std::string m_name;

            std::vector<uint32_t> m_indices;
            std::vector<Vertex> m_vertices;
        };

        explicit Mesh(const Builder &builder);
        ~Mesh();

        Mesh(const Mesh &) = delete;
        Mesh &operator=(const Mesh &) = delete;

        [[nodiscard]] const std::string &Name() const { return m_name; }

        void Bind(const vk::CommandBuffer &commandBuffer) const;
        void Draw(const vk::CommandBuffer &commandBuffer, uint32_t instanceCount) const;

    private:
        std::string m_name;

        std::unique_ptr<Memory::Buffer> m_indexBuffer;
        std::unique_ptr<Memory::Buffer> m_vertexBuffer;

        void CreateVertexBuffer(const std::vector<Vertex> &vertices);
        void CreateIndexBuffer(const std::vector<uint32_t> &indices);

    public:
        static const Mesh *Cube();
        static const Mesh *Sphere();
        static std::unique_ptr<Mesh> Frustum(float fov, float aspect, float near, float far);
        static std::unique_ptr<Mesh> Cuboid(float left, float right, float bottom, float top, float near, float far);
    private:
        inline static std::unordered_map<std::string, boost::uuids::uuid> m_meshes;
    };
}