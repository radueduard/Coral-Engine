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
	std::unique_ptr<Mesh> Sphere(int segments = 16, int rings = 16);
	std::unique_ptr<Mesh> Cylinder(int segments = 16, bool caps = true);
	std::unique_ptr<Mesh> Cone(int segments = 16, bool cap = true);
	std::unique_ptr<Mesh> Prism();
	// std::unique_ptr<Mesh> Plane(int patchSize = 1);
	std::unique_ptr<Mesh> Frustum(const ECS::Camera *camera);
}