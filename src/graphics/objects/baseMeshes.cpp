//
// Created by radue on 12/1/2024.
//

#include <glm/ext/scalar_constants.hpp>

#include "baseMeshes.h"
#include "ecs/components/camera.h"
#include "ecs/components/renderTarget.h"

namespace Coral::Graphics {
    std::unique_ptr<Mesh> Cube(const int patchSize) {
	    // Create a cube mesh with 24 vertices and 36 indices

    	auto builder = Mesh::Builder(boost::uuids::string_generator()("00000000-0000-0000-0000-000000000001"));
    	builder
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

    std::unique_ptr<Mesh> Sphere(const int segments, const int rings) {
        auto sphere = Mesh::Builder(boost::uuids::string_generator()("00000000-0000-0000-0000-000000000002"));
    	sphere
            .Name("Sphere");

        for (int i = 0; i <= rings; i++) {
            const float theta = static_cast<float>(i) * glm::pi<float>() / static_cast<float>(rings);
            for (int j = 0; j <= segments; j++) {
                const float phi = static_cast<float>(j) * 2.0f * glm::pi<float>() / static_cast<float>(segments);

            	Math::Vector3f position = {
            		sin(theta) * cos(phi),
            		cos(theta),
            		sin(theta) * sin(phi)
            	};
            	Math::Vector3f normal = position.Normalized();
                Math::Vector3f tangent = { -sin(phi), 0.0f, cos(phi) };

                Math::Vector2f texCoord = {
	                static_cast<float>(j) / static_cast<float>(segments),
                	static_cast<float>(i) / static_cast<float>(rings)
                };
                Math::Vector4f tangent4 = Math::Vector4(tangent, 1.f);
                sphere.AddVertex({position, normal, tangent4, texCoord});
            }
        }

        for (int i = 0; i < rings; i++) {
            for (int j = 0; j < segments; j++) {
                const int first = i * (segments + 1) + j;
                const int second = first + segments + 1;

                sphere.AddIndex(first).AddIndex(first + 1).AddIndex(second);
                sphere.AddIndex(second + 1).AddIndex(second).AddIndex(first + 1);
            }
        }
        return sphere.Build();
    }

	std::unique_ptr<Mesh> Cylinder(int segments, bool caps) {
    	auto cylinder = Mesh::Builder(boost::uuids::string_generator()("00000000-0000-0000-0000-000000000003"));
    	cylinder
			.Name("Cylinder");
    	for (int i = 0; i <= segments; i++) {
			const float theta = static_cast<float>(i) * 2.0f * glm::pi<float>() / static_cast<float>(segments);

			Math::Vector3f topPosition = { cos(theta), 1.0f, sin(theta) };
			Math::Vector3f bottomPosition = { cos(theta), -1.0f, sin(theta) };
			Math::Vector3f normal = { cos(theta), 0.0f, sin(theta) };
			Math::Vector3f tangent = { -sin(theta), 0.0f, cos(theta) };

			Math::Vector2f topTexCoord = {
				static_cast<float>(i) / static_cast<float>(segments),
				0.0f
			};
			Math::Vector2f bottomTexCoord = {
				static_cast<float>(i) / static_cast<float>(segments),
				1.0f
			};
			Math::Vector4f tangent4 = Math::Vector4(tangent, 1.f);
			cylinder.AddVertex({topPosition, normal, tangent4, topTexCoord});
			cylinder.AddVertex({bottomPosition, normal, tangent4, bottomTexCoord});
		}

    	if (caps) {
    		// duplicate vertices for caps
    		for (int i = 0; i <= segments; i++) {
    			const float theta = static_cast<float>(i) * 2.0f * glm::pi<float>() / static_cast<float>(segments);
    			Math::Vector3f topPosition = { cos(theta), 1.0f, sin(theta) };
    			Math::Vector3f bottomPosition = { cos(theta), -1.0f, sin(theta) };
    			Math::Vector3f topNormal = { 0.0f, 1.0f, 0.0f };
    			Math::Vector3f bottomNormal = { 0.0f, -1.0f, 0.0f };
    			Math::Vector3f tangent = { -sin(theta), 0.0f, cos(theta) };
    			Math::Vector2f topTexCoord = {
    				(cos(theta) + 1.0f) * 0.5f,
					(sin(theta) + 1.0f) * 0.5f
				};
    			Math::Vector2f bottomTexCoord = {
    				(cos(theta) + 1.0f) * 0.5f,
					(sin(theta) + 1.0f) * 0.5f
				};
    			Math::Vector4f tangent4 = Math::Vector4(tangent, 1.f);
    			cylinder.AddVertex({topPosition, topNormal, tangent4,  topTexCoord});
    			cylinder.AddVertex({bottomPosition, bottomNormal, tangent4, bottomTexCoord});
    		}
    		const int topCenterIndex = (segments + 1) * 2;
			const int bottomCenterIndex = topCenterIndex + 1;
    		cylinder.AddVertex({{0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.5f, 0.5f}});
			cylinder.AddVertex({{0.0f, -1.0f, 0.0f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.5f, 0.5f}});

			for (int i = 0; i < segments; i++) {
				const int topVertexIndex = (segments + 1) * 2 + 2 + i * 2;
				const int bottomVertexIndex = topVertexIndex + 1;
				const int nextTopVertexIndex = (i == segments - 1) ? (segments + 1) * 2 + 2 : topVertexIndex + 2;
				const int nextBottomVertexIndex = nextTopVertexIndex + 1;

				// Top cap
				cylinder.AddIndex(topCenterIndex).AddIndex(topVertexIndex).AddIndex(nextTopVertexIndex);
				// Bottom cap
				cylinder.AddIndex(bottomCenterIndex).AddIndex(nextBottomVertexIndex).AddIndex(bottomVertexIndex);
			}
    	}

		for (int i = 0; i < segments; i++) {
			const int top1 = i * 2;
			const int bottom1 = top1 + 1;
			const int top2 = ((i + 1) % (segments + 1)) * 2;
			const int bottom2 = top2 + 1;

			cylinder.AddIndex(top1).AddIndex(bottom1).AddIndex(top2);
			cylinder.AddIndex(bottom2).AddIndex(top2).AddIndex(bottom1);
		}

		return cylinder.Build();
	}

	std::unique_ptr<Mesh> Cone(int segments, bool cap) {
    	auto cone = Mesh::Builder(boost::uuids::string_generator()("00000000-0000-0000-0000-000000000005"));
		cone
			.Name("Cone");

    	Math::Vector3f apexPosition = { 0.0f, 1.0f, 0.0f };
		Math::Vector3f apexNormal = { 0.0f, 1.0f, 0.0f };
		Math::Vector4f apexTangent = { 1.0f, 0.0f, 0.0f, 1.0f };
		Math::Vector2f apexTexCoord = { 0.5f, 0.0f };
		cone.AddVertex({ apexPosition, apexNormal, apexTangent, apexTexCoord });

		for (int i = 0; i <= segments; i++) {
			const float theta = static_cast<float>(i) * 2.0f * glm::pi<float>() / static_cast<float>(segments);
			Math::Vector3f basePosition = { cos(theta), -1.0f, sin(theta) };
			Math::Vector3f baseNormal = Math::Vector3f { cos(theta), 0.0f, sin(theta) }.Normalized();
			Math::Vector3f tangent = { -sin(theta), 0.0f, cos(theta) };
			Math::Vector2f baseTexCoord = {
				(static_cast<float>(i) / static_cast<float>(segments)),
				1.0f
			};
			Math::Vector4f tangent4 = Math::Vector4(tangent, 1.f);
			cone.AddVertex({ basePosition, baseNormal, tangent4, baseTexCoord });
		}

		for (int i = 1; i <= segments; i++) {
			cone.AddIndex(0).AddIndex(i).AddIndex(i + 1);
		}

		if (cap) {
			const int baseCenterIndex = segments + 2;
			cone.AddVertex({ {0.0f, -1.0f, 0.0f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.5f, 0.5f} });

			for (int i = 1; i <= segments; i++) {
				const int currentBaseIndex = i;
				const int nextBaseIndex = (i % segments) + 1;
				cone.AddIndex(baseCenterIndex).AddIndex(nextBaseIndex).AddIndex(currentBaseIndex);
			}
		}

		return cone.Build();
	}

	std::unique_ptr<Mesh> Prism() {
    	auto prism = Mesh::Builder(boost::uuids::string_generator()("00000000-0000-0000-0000-000000000004"));
		prism
			.Name("Prism");

    	Math::Vector2f p1 = Math::Vector2f { 0.0f, 1.0f };
    	Math::Vector2f p2 = Math::Vector2f { -0.866f, -0.5f };
    	Math::Vector2f p3 = Math::Vector2f { 0.866f, -0.5f };

    	prism
    		// Top face
    		.AddVertex({ {p1.x, 1.0f, p1.y}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.5f, 1.0f} })
    		.AddVertex({ {p2.x, 1.0f, p2.y}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f} })
			.AddVertex({ {p3.x, 1.0f, p3.y}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 0.0f} })
			// Bottom face
			.AddVertex({ {p1.x, -1.0f, p1.y}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.5f, 1.0f} })
			.AddVertex({ {p2.x, -1.0f, p2.y}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f} })
			.AddVertex({ {p3.x, -1.0f, p3.y}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 0.0f} });

    	// Top face
    	prism.AddIndex(0).AddIndex(1).AddIndex(2);
		// Bottom face
    	prism.AddIndex(5).AddIndex(4).AddIndex(3);
    	// Side faces
    	prism.AddIndex(0).AddIndex(3).AddIndex(1);
		prism.AddIndex(4).AddIndex(1).AddIndex(3);
    	prism.AddIndex(1).AddIndex(4).AddIndex(2);
		prism.AddIndex(5).AddIndex(2).AddIndex(4);
		prism.AddIndex(2).AddIndex(5).AddIndex(0);
    	prism.AddIndex(3).AddIndex(0).AddIndex(5);

    	return prism.Build();
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

