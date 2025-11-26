//
// Created by radue on 10/20/2024.
//

#pragma once

#include <memory>
#include <set>
#include <vector>

#include <boost/uuid/uuid.hpp>
#include <vulkan/vulkan.hpp>

#include "color/color.h"
#include "math/aabb.h"
#include "math/vector.h"

namespace Coral::Memory {
	class Buffer;
}

namespace Coral::Shader {
	struct InOut;
	class Shader;
}

namespace Coral::Graphics {
    struct Vertex {
        enum class Attribute : u16 {
            POSITION = 1 << 0,
            NORMAL = 1 << 1,
            TANGENT = 1 << 2,
            TEXCOORD = 1 << 3,
            TEXCOORD1 = 1 << 4,
            COLOR = 1 << 5
        };

        Math::Vector3f position = {0.0f, 0.0f, 0.0f};
        Math::Vector3f normal = {0.0f, 0.0f, 0.0f};
        Math::Vector4f tangent = {0.0f, 0.0f, 0.0f, 1.0f};
        Math::Vector2f texCoord0 = { 0.0f, 0.0f };
        Math::Vector2f texCoord1 = {0.0f, 0.0f};
        Color color0 = Colors::white;

        static std::vector<vk::VertexInputBindingDescription> BindingDescriptions();

		static std::vector<vk::VertexInputAttributeDescription> AttributeDescriptions(const std::set<Shader::InOut>& inputAnalysis);

	private:
        static size_t Offset(const Attribute attribute);
	};

    class Mesh {
    public:
        class Builder {
            friend class Mesh;
        public:
            explicit Builder(const boost::uuids::uuid &uuid);
			~Builder();

            Builder& Name(const std::string &name);

			Builder& AddVertex(Vertex vertex);

			Builder& AddIndex(u32 index);

			Builder& AABB(const Math::AABB &aabb);

			std::unique_ptr<Mesh> Build();

		private:
            UUID m_uuid;
            String m_name;
        	std::optional<Math::AABB> m_aabb = std::nullopt;
            std::vector<Vertex> m_vertices;
            std::vector<u32> m_indices;
        };

        explicit Mesh(Builder &builder);

		~Mesh();

        [[nodiscard]] const UUID &Id() const;
		[[nodiscard]] const std::string &Name() const;

		void Bind(const vk::CommandBuffer &commandBuffer) const;

		void Draw(const vk::CommandBuffer &commandBuffer, const uint32_t instanceCount = 1) const;

	private:
        UUID m_uuid;
        String m_name;
    	Math::AABB m_aabb;
        std::unique_ptr<Memory::Buffer> m_indexBuffer;
        std::unique_ptr<Memory::Buffer> m_vertexBuffer;

        void CreateVertexBuffer(std::vector<Vertex> &vertices);

		void CreateIndexBuffer(std::vector<u32> &indices);
	};
}