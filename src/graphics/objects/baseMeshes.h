//
// Created by radue on 2/27/2025.
//


#pragma once

#include <memory>
#include "mesh.h"


namespace Coral {
	namespace ECS {
		class Camera;
	}

	std::unique_ptr<Graphics::Mesh> Cube();
	std::unique_ptr<Graphics::Mesh> Sphere();
	std::unique_ptr<Graphics::Mesh> Frustum(const ECS::Camera *camera);
}