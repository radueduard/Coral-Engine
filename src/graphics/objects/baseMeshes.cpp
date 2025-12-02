//
// Created by radue on 12/1/2024.
//

#include <glm/ext/scalar_constants.hpp>

#include "baseMeshes.h"
#include "ecs/components/camera.h"
#include "ecs/components/RenderTarget.h"

namespace Coral::Graphics {
    std::unique_ptr<Mesh> Cube(const int patchSize) {
	    // Create a cube mesh with 24 vertices and 36 indices

    	auto builder = Mesh::Builder(boost::uuids::string_generator()("00000000-0000-0000-0000-000000000001"))
			.Name("Cube")
			// Front face
			.AddVertex({{1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}})
			.AddVertex({{1.0f, -1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}})
			.AddVertex({{-1.0f, -1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}})
			.AddVertex({{-1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}})
			// Back face
			.AddVertex({{1.0f, 1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {-1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}})
			.AddVertex({{1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {-1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}})
			.AddVertex({{-1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {-1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}})
			.AddVertex({{-1.0f, 1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {-1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}})
			// Right face
			.AddVertex({{1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f, 1.0f}, {1.0f, 1.0f}})
			.AddVertex({{1.0f, -1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f, 1.0f}, {1.0f, 0.0f}})
			.AddVertex({{1.0f, -1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f, 1.0f}, {0.0f, 0.0f}})
			.AddVertex({{1.0f, 1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f, 1.0f}, {0.0f, 1.0f}})
			// Left face
			.AddVertex({{-1.0f, 1.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}, {0.0f, 1.0f}})
			.AddVertex({{-1.0f, -1.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}, {0.0f, 0.0f}})
			.AddVertex({{-1.0f, -1.0f, -1.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}, {1.0f, 0.0f}})
			.AddVertex({{-1.0f, 1.0f, -1.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}, {1.0f, 1.0f}})
			// Top face
			.AddVertex({{1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}})
			.AddVertex({{1.0f, 1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}})
			.AddVertex({{-1.0f, 1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}})
			.AddVertex({{-1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}})
			// Bottom face
			.AddVertex({{1.0f, -1.0f, 1.0f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}})
			.AddVertex({{1.0f, -1.0f, -1.0f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}})
			.AddVertex({{-1.0f, -1.0f, -1.0f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}})
			.AddVertex({{-1.0f, -1.0f, 1.0f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}});
    	// Indices for 6 faces (2 triangles per face)
    	if (patchSize == 3) {
    		builder
    			// Front face
				.AddIndex(0).AddIndex(2).AddIndex(1)
				.AddIndex(0).AddIndex(3).AddIndex(2)
				// Back face
				.AddIndex(4).AddIndex(6).AddIndex(7)
				.AddIndex(4).AddIndex(5).AddIndex(6)
				// Right face
				.AddIndex(8).AddIndex(9).AddIndex(10)
				.AddIndex(11).AddIndex(8).AddIndex(10)
				// Left face
				.AddIndex(12).AddIndex(15).AddIndex(14)
				.AddIndex(13).AddIndex(12).AddIndex(14)
				// Top face
				.AddIndex(16).AddIndex(17).AddIndex(18)
				.AddIndex(19).AddIndex(16).AddIndex(18)
				// Bottom face
				.AddIndex(20).AddIndex(23).AddIndex(22)
				.AddIndex(21).AddIndex(20).AddIndex(22);
    	} else if (patchSize == 4) {
    		builder
    			// Front face
    			.AddIndex(0).AddIndex(1).AddIndex(2).AddIndex(3)
    			// Back face
    			.AddIndex(4).AddIndex(5).AddIndex(6).AddIndex(7)
				// Right face
    			.AddIndex(8).AddIndex(9).AddIndex(10).AddIndex(11)
				// Left face
				.AddIndex(12).AddIndex(13).AddIndex(14).AddIndex(15)
    			// Top face
				.AddIndex(16).AddIndex(17).AddIndex(18).AddIndex(19)
    			// Bottom face
				.AddIndex(20).AddIndex(21).AddIndex(22).AddIndex(23);
    	}
    	return builder.Build();
    }

    std::unique_ptr<Mesh> Sphere() {
        auto sphere = Mesh::Builder(boost::uuids::string_generator()("00000000-0000-0000-0000-000000000002"))
            .Name("Sphere");

        int density = 20;
        for (int i = 0; i <= density; i++) {
            const float theta = static_cast<float>(i) * glm::pi<float>() / static_cast<float>(density);
            for (int j = 0; j <= density; j++) {
                const float phi = static_cast<float>(j) * 2.0f * glm::pi<float>() / static_cast<float>(density);

            	Math::Vector3f position = {
            		sin(theta) * cos(phi),
            		cos(theta),
            		sin(theta) * sin(phi)
            	};
            	Math::Vector3f normal = position.Normalized();

            	f32 r = position.Length();
            	f32 tangentTheta = std::acos(position.z / r);
            	f32 tangentPhi = std::atan2(position.y, position.x);

                Math::Vector3f tangent =Math::Vector3f{
					-sin(tangentTheta) * cos(tangentPhi),
					sin(tangentTheta) * sin(tangentPhi),
					cos(tangentTheta)
				}.Normalized();

                Math::Vector3f bitangent = normal.Cross(tangent).Normalized();
                Math::Vector2f texCoord = {static_cast<float>(j) / static_cast<float>(density), static_cast<float>(i) / static_cast<float>(density)};
                Math::Vector4f tangent4 = Math::Vector4(tangent, normal.Cross(tangent).Dot(bitangent) < 0.0f ? -1.0f : 1.0f);
                sphere.AddVertex({position, normal, tangent4, texCoord});
            }
        }

        for (int i = 0; i < density; i++) {
            for (int j = 0; j < density; j++) {
                const int first = i * (density + 1) + j;
                const int second = first + density + 1;

                sphere.AddIndex(first).AddIndex(first + 1).AddIndex(second);
                sphere.AddIndex(second + 1).AddIndex(second).AddIndex(first + 1);
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

        Math::Vector3f nearCenter = { 0.0f, 0.0f, -near };
        Math::Vector3f farCenter = { 0.0f, 0.0f, -far };

        Math::Vector3<f32> nearTopLeft = nearCenter + Math::Vector3f { -nearWidth, nearHeight,  0.0f };
        Math::Vector3<f32> nearTopRight = nearCenter + Math::Vector3f { nearWidth, nearHeight, 0.0f };
        Math::Vector3<f32> nearBottomLeft = nearCenter + Math::Vector3f { -nearWidth, -nearHeight, 0.0f };
        Math::Vector3<f32> nearBottomRight = nearCenter + Math::Vector3f { nearWidth, -nearHeight, 0.0f	};
        Math::Vector3<f32> farTopLeft = farCenter + Math::Vector3f { -farWidth, farHeight, 0.0f };
        Math::Vector3<f32> farTopRight = farCenter + Math::Vector3f { farWidth, farHeight, 0.0f };
        Math::Vector3<f32> farBottomLeft = farCenter + Math::Vector3f { -farWidth, -farHeight, 0.0f };
        Math::Vector3<f32> farBottomRight = farCenter + Math::Vector3f { farWidth, -farHeight, 0.0f };

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

