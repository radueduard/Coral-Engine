//
// Created by radue on 2/27/2025.
//


#pragma once

#include <memory>
#include "mesh.h"


namespace mgv {
	class Camera;

	std::unique_ptr<Mesh> Cube();
	std::unique_ptr<Mesh> Sphere();
	std::unique_ptr<Mesh> Frustum(const Camera *camera);
}