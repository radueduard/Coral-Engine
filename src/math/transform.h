//
// Created by radue on 4/21/2025.
//
#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>

#include "constants.h"
#include "matrix.h"
#include "quaternion.h"
#include "vector.h"

namespace Coral::Math {
    inline Matrix4<f32> Translate(const Vector3<f32>& position) {
        auto result = Matrix4<f32>::Identity();
        result[3] = Vector4(position, 1.0f);
        return result;
    }

    inline Matrix4<f32> Scale(const Vector3<f32>& scale) {
        auto result = Matrix4<f32>::Identity();
        result[0][0] = scale[0];
        result[1][1] = scale[1];
        result[2][2] = scale[2];
        return result;
    }

    inline Matrix4<f32> Rotate(const Vector3<f32>& eulerAngles) {
        return Quaternion(eulerAngles).ToMatrix();
    }

    inline Matrix4<f32> Rotate(const Quaternion<f32>& rotation) {
        return rotation.ToMatrix();
    }

    inline Vector3<f32> Rotate(const Quaternion<f32>& rotation, const Vector3<f32>& direction) {
        return Vector3(glm::rotate(
			reinterpret_cast<const glm::quat&>(rotation),
			reinterpret_cast<const glm::vec3&>(direction)));
    }

    /**
     * @brief Function that calculates the projection matrix of a camera
     *
     * @param fov the field of view value in radians
     * @param resolution the resolution of the screen
     * @param planes the near (x) and far (y) planes of the camera
     * @return the projection matrix of the camera
     */
    inline Matrix4<f32> Perspective(const f32 fov, const Vector2<f32> resolution, const Vector2<f32> planes) {
        return static_cast<Matrix4<f32>>(glm::perspective(fov, resolution.x / resolution.y, planes.x, planes.y));
    }

    /**
     * @brief Function that calculates the orthographic projection matrix of a camera
     *
     * @param oxPlanes the left (x) and right (y)planes of the projected volume
     * @param oyPlanes the bottom (x) and top (y) planes of the projected volume
     * @param ozPlanes the near (x) and far (y) planes of the projected volume
     * @return the orthographic projection matrix
     */
    inline Matrix4<f32> Orthographic(Vector2<f32> oxPlanes, Vector2<f32> oyPlanes, Vector2<f32> ozPlanes) {
        return static_cast<Matrix4<f32>>(glm::ortho(oxPlanes.x, oxPlanes.y, oyPlanes.x, oyPlanes.y, ozPlanes.x, ozPlanes.y));
    }

    /**
     * @brief Function that calculates the view matrix of a camera
     *
     * @param position the position of the camera
     * @param target the target point of the camera
     * @param up the up vector of the camera
     * @return the view matrix of the camera
     */
    inline Matrix4<f32> LookAt(const Vector3<f32>& position, const Vector3<f32>& target, const Vector3<f32>& up) {
        return static_cast<Matrix4<f32>>(glm::lookAt(
            reinterpret_cast<const glm::vec3&>(position),
            reinterpret_cast<const glm::vec3&>(target),
            reinterpret_cast<const glm::vec3&>(up)));
    }

    inline Quaternion<f32> LookAt(const Vector3<f32>& direction, const Vector3<f32>& up) {
        return Quaternion(glm::quatLookAt(
            reinterpret_cast<const glm::vec3&>(direction),
            reinterpret_cast<const glm::vec3&>(up)));
    }

	inline Vector3f Direction(const Vector3<f32>& eulerAngles) {
    	return Vector3f {
    		glm::cos(eulerAngles.y) * glm::cos(eulerAngles.x),
			glm::sin(eulerAngles.x),
			glm::sin(eulerAngles.y) * glm::cos(eulerAngles.x)
		}.Normalized();
    }
}
