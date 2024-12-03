//
// Created by radue on 12/1/2024.
//

#include <glm/ext/scalar_constants.hpp>

#include "mesh.h"
#include "assets/manager.h"

std::unordered_map<std::string, boost::uuids::uuid> mgv::Mesh::m_meshes = {};

const mgv::Mesh *mgv::Mesh::Cube() {
    if (!m_meshes.contains("Cube")) {
        auto cube = Builder("Cube")
            .AddVertex({{-1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}})
            .AddVertex({{ 1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}})
            .AddVertex({{ 1.0f,  1.0f, -1.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}})
            .AddVertex({{-1.0f,  1.0f, -1.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}})
            .AddVertex({{-1.0f, -1.0f,  1.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}})
            .AddVertex({{ 1.0f, -1.0f,  1.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}})
            .AddVertex({{ 1.0f,  1.0f,  1.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}})
            .AddVertex({{-1.0f,  1.0f,  1.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}})
            .AddIndex(0).AddIndex(1).AddIndex(2).AddIndex(2).AddIndex(3).AddIndex(0)
            .AddIndex(4).AddIndex(5).AddIndex(6).AddIndex(6).AddIndex(7).AddIndex(4)
            .AddIndex(0).AddIndex(4).AddIndex(7).AddIndex(7).AddIndex(3).AddIndex(0)
            .AddIndex(1).AddIndex(5).AddIndex(6).AddIndex(6).AddIndex(2).AddIndex(1)
            .AddIndex(0).AddIndex(1).AddIndex(5).AddIndex(5).AddIndex(4).AddIndex(0)
            .AddIndex(2).AddIndex(6).AddIndex(7).AddIndex(7).AddIndex(3).AddIndex(2)
            .Build(Core::Device::Get());
        m_meshes["Cube"] = Asset::Manager::AddMesh(std::move(cube));
    }

    return Asset::Manager::GetMesh(m_meshes["Cube"]);
}

const mgv::Mesh*mgv:: Mesh::Sphere() {
    if (!m_meshes.contains("Sphere")) {
        auto sphere = Builder("Sphere");

        int density = 5;
        for (int i = 0; i <= density; i++) {
            const float theta = static_cast<float>(i) * glm::pi<float>() / static_cast<float>(density);
            for (int j = 0; j <= density; j++) {
                const float phi = static_cast<float>(j) * 2.0f * glm::pi<float>() / static_cast<float>(density);

                glm::vec3 position = {
                    sin(theta) * cos(phi),
                    cos(theta),
                    sin(theta) * sin(phi)
                };

                glm::vec3 normal = glm::normalize(position);
                glm::vec3 tangent = glm::normalize(glm::cross(normal, glm::vec3(0.0f, 1.0f, 0.0f)));
                glm::vec3 bitangent = glm::cross(tangent, normal);
                glm::vec2 texCoord = {static_cast<float>(j) / static_cast<float>(density), static_cast<float>(i) / static_cast<float>(density)};
                glm::vec4 tangent4 = {tangent, glm::dot(glm::cross(normal, tangent), bitangent) < 0.0f ? -1.0f : 1.0f};
                sphere.AddVertex({position, normal, tangent4, texCoord});
            }
        }

        for (int i = 0; i < density; i++) {
            for (int j = 0; j < density; j++) {
                const int first = i * (density + 1) + j;
                const int second = first + density + 1;

                sphere.AddIndex(first).AddIndex(second).AddIndex(first + 1);
                sphere.AddIndex(second).AddIndex(second + 1).AddIndex(first + 1);
            }
        }

        m_meshes["Sphere"] = Asset::Manager::AddMesh(std::move(sphere.Build(Core::Device::Get())));
    }
    return Asset::Manager::GetMesh(m_meshes["Sphere"]);
}