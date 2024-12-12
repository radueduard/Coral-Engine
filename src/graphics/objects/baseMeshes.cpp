//
// Created by radue on 12/1/2024.
//

#include <glm/ext/scalar_constants.hpp>

#include "mesh.h"
#include "assets/manager.h"
#include "components/camera.h"

namespace mgv {
    const Mesh *Mesh::Cube() {
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
                .Build();
            m_meshes["Cube"] = Asset::Manager::AddMesh(std::move(cube));
        }

        return Asset::Manager::GetMesh(m_meshes["Cube"]);
    }

    const Mesh *Mesh::Sphere() {
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

            m_meshes["Sphere"] = Asset::Manager::AddMesh(std::move(sphere.Build()));
        }
        return Asset::Manager::GetMesh(m_meshes["Sphere"]);
    }

    std::unique_ptr<Mesh> Mesh::Frustum(const Camera *camera) {
        const float near = camera->m_projectionData.perspective.near;
        const float far = camera->m_projectionData.perspective.near + 5.0f;
        const float aspect = static_cast<float>(camera->m_viewportSize.x) / static_cast<float>(camera->m_viewportSize.y);
        const float fov = camera->m_projectionData.perspective.fov;

        float tanHalfFov = tan(glm::radians(fov) / 2.0f);
        float nearHeight = tanHalfFov * near;
        float nearWidth = nearHeight * aspect;
        float farHeight = tanHalfFov * far;
        float farWidth = farHeight * aspect;

        glm::vec3 nearCenter = glm::vec3(0.0f, 0.0f, -near);
        glm::vec3 farCenter = glm::vec3(0.0f, 0.0f, -far);

        glm::vec3 nearTopLeft = nearCenter + glm::vec3(-nearWidth, nearHeight,  0.0f);
        glm::vec3 nearTopRight = nearCenter + glm::vec3(nearWidth, nearHeight, 0.0f);
        glm::vec3 nearBottomLeft = nearCenter + glm::vec3(-nearWidth, -nearHeight, 0.0f);
        glm::vec3 nearBottomRight = nearCenter + glm::vec3(nearWidth, -nearHeight, 0.0f);
        glm::vec3 farTopLeft = farCenter + glm::vec3(-farWidth, farHeight, 0.0f);
        glm::vec3 farTopRight = farCenter + glm::vec3(farWidth, farHeight, 0.0f);
        glm::vec3 farBottomLeft = farCenter + glm::vec3(-farWidth, -farHeight, 0.0f);
        glm::vec3 farBottomRight = farCenter + glm::vec3(farWidth, -farHeight, 0.0f);

        return Builder("Perspective frustum volume")
            .AddVertex({nearTopLeft, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}})
            .AddVertex({nearTopRight, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}})
            .AddVertex({nearBottomLeft, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}})
            .AddVertex({nearBottomRight, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}})
            .AddVertex({farTopLeft, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}})
            .AddVertex({farTopRight, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}})
            .AddVertex({farBottomLeft, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}})
            .AddVertex({farBottomRight, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}})
            .AddIndex(0).AddIndex(1).AddIndex(2).AddIndex(2).AddIndex(3).AddIndex(1)
            .AddIndex(4).AddIndex(5).AddIndex(6).AddIndex(6).AddIndex(7).AddIndex(5)
            .AddIndex(0).AddIndex(1).AddIndex(5).AddIndex(5).AddIndex(4).AddIndex(0)
            .AddIndex(2).AddIndex(3).AddIndex(7).AddIndex(7).AddIndex(6).AddIndex(2)
            .AddIndex(0).AddIndex(2).AddIndex(6).AddIndex(6).AddIndex(4).AddIndex(0)
            .AddIndex(1).AddIndex(3).AddIndex(7).AddIndex(7).AddIndex(5).AddIndex(1)
            .Build();
    }
}

