//
// Created by radue on 2/27/2025.
//


#pragma once

#include <memory>
#include "mesh.h"

namespace Coral::ECS {
	class Camera;
}

namespace Coral::Graphics {

	std::unique_ptr<Mesh> Cube(int patchSize = 3);
	std::unique_ptr<Mesh> Sphere();
	std::unique_ptr<Mesh> Frustum(const ECS::Camera *camera);
}