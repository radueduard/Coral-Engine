//
// Created by radue on 12/1/2024.
//

#include <glm/ext/scalar_constants.hpp>

#include "baseMeshes.h"
#include "ecs/components/camera.h"
#include "ecs/components/RenderTarget.h"

namespace Coral::Graphics {
    std::unique_ptr<Mesh> Cube() {
        return Mesh::Builder(boost::uuids::random_generator()())
            .Name("Cube")
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
    }

    std::unique_ptr<Mesh> Sphere() {
        auto sphere = Mesh::Builder(boost::uuids::random_generator()())
            .Name("Sphere");

        int density = 5;
        for (int i = 0; i <= density; i++) {
            const float theta = static_cast<float>(i) * glm::pi<float>() / static_cast<float>(density);
            for (int j = 0; j <= density; j++) {
                const float phi = static_cast<float>(j) * 2.0f * glm::pi<float>() / static_cast<float>(density);

               Math::Vector3 position = {
                    sin(theta) * cos(phi),
                    cos(theta),
                    sin(theta) * sin(phi)
                };

                Math::Vector3<f32> normal = position.Normalize();
                Math::Vector3<f32> tangent = normal.Cross(Math::Vector3 { 0.0f, 1.0f, 0.0f }).Normalize();
                Math::Vector3<f32> bitangent = normal.Cross(tangent).Normalize();
                Math::Vector2 texCoord = {static_cast<float>(j) / static_cast<float>(density), static_cast<float>(i) / static_cast<float>(density)};
                Math::Vector4<f32> tangent4 = Math::Vector4(tangent, normal.Cross(tangent).Dot(bitangent) < 0.0f ? -1.0f : 1.0f);
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
        return sphere.Build();
    }

    std::unique_ptr<Mesh> Frustum(ECS::Camera *camera) {
        const float near = camera->GetProjectionData().data.perspective.near;
        const float far = camera->GetProjectionData().data.perspective.near + 5.0f;
        const float aspect = camera->AspectRatio();
        const float fov = camera->GetProjectionData().data.perspective.fov;

        float tanHalfFov = tan(glm::radians(fov) / 2.0f);
        float nearHeight = tanHalfFov * near;
        float nearWidth = nearHeight * aspect;
        float farHeight = tanHalfFov * far;
        float farWidth = farHeight * aspect;

        Math::Vector3 nearCenter = { 0.0f, 0.0f, -near };
        Math::Vector3 farCenter = { 0.0f, 0.0f, -far };

        Math::Vector3<f32> nearTopLeft = nearCenter + Math::Vector3 { -nearWidth, nearHeight,  0.0f };
        Math::Vector3<f32> nearTopRight = nearCenter + Math::Vector3 { nearWidth, nearHeight, 0.0f };
        Math::Vector3<f32> nearBottomLeft = nearCenter + Math::Vector3 { -nearWidth, -nearHeight, 0.0f };
        Math::Vector3<f32> nearBottomRight = nearCenter + Math::Vector3 { nearWidth, -nearHeight, 0.0f	};
        Math::Vector3<f32> farTopLeft = farCenter + Math::Vector3 { -farWidth, farHeight, 0.0f };
        Math::Vector3<f32> farTopRight = farCenter + Math::Vector3 { farWidth, farHeight, 0.0f };
        Math::Vector3<f32> farBottomLeft = farCenter + Math::Vector3 { -farWidth, -farHeight, 0.0f };
        Math::Vector3<f32> farBottomRight = farCenter + Math::Vector3 { farWidth, -farHeight, 0.0f };

        return Mesh::Builder(boost::uuids::random_generator()())
            .Name("Frustum Volume")
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

