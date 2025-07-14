//
// Created by radue on 14/07/2025.
//
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

export module math.transform;
export import math.vector;
export import math.matrix;
export import math.quaternion;




namespace Coral::Math {
    export template <typename T = f32> requires std::is_floating_point_v<T>
    constexpr Matrix4<T> Translate(const Vector3<T>& position) {
        auto result = Matrix4<T>::Identity();
        result[3] = Vector4(position, static_cast<T>(1));
        return result;
    }

    export template <typename T = f32> requires std::is_floating_point_v<T>
    constexpr Matrix4<T> Scale(const Vector3<T>& scale) {
        auto result = Matrix4<T>::Identity();
        result[0][0] = scale[0];
        result[1][1] = scale[1];
        result[2][2] = scale[2];
        return result;
    }

    export template <typename T = f32> requires std::is_floating_point_v<T>
    constexpr Matrix4<T> Rotate(const Vector3<T>& eulerAngles) {
        return Quaternion(eulerAngles).ToMatrix();
    }

    export template <typename T = f32> requires std::is_floating_point_v<T>
    constexpr Matrix4<T> Rotate(const Quaternion<T>& rotation) {
        return rotation.ToMatrix();
    }

    export template <typename T = f32> requires std::is_floating_point_v<T>
    constexpr Vector3<T> Rotate(const Quaternion<T>& rotation, const Vector3<T>& direction) {
        return Vector3(glm::rotate(
			reinterpret_cast<const glm::qua<T>&>(rotation),
			reinterpret_cast<const glm::vec<3, T>&>(direction)));
    }

    /**
     * @brief Function that calculates the projection matrix of a camera
     *
     * @param fov the field of view value in radians
     * @param resolution the resolution of the screen
     * @param planes the near (x) and far (y) planes of the camera
     * @return the projection matrix of the camera
     */
    export template <typename T = f32> requires std::is_floating_point_v<T>
    constexpr Matrix4<T> Perspective(const T fov, const Vector2<T> resolution, const Vector2<T> planes) {
        return static_cast<Matrix4<T>>(glm::perspective(fov, resolution.x / resolution.y, planes.x, planes.y));
    }

    /**
     * @brief Function that calculates the orthographic projection matrix of a camera
     *
     * @param oxPlanes the left (x) and right (y)planes of the projected volume
     * @param oyPlanes the bottom (x) and top (y) planes of the projected volume
     * @param ozPlanes the near (x) and far (y) planes of the projected volume
     * @return the orthographic projection matrix
     */
    export template <typename T = f32> requires std::is_floating_point_v<T>
    constexpr Matrix4<T> Orthographic(Vector2<T> oxPlanes, Vector2<T> oyPlanes, Vector2<T> ozPlanes) {
        return static_cast<Matrix4<T>>(glm::ortho(oxPlanes.x, oxPlanes.y, oyPlanes.x, oyPlanes.y, ozPlanes.x, ozPlanes.y));
    }

    /**
     * @brief Function that calculates the view matrix of a camera
     *
     * @param position the position of the camera
     * @param target the target point of the camera
     * @param up the up vector of the camera
     * @return the view matrix of the camera
     */
    export template <typename T = f32> requires std::is_floating_point_v<T>
    constexpr Matrix4<T> LookAt(const Vector3<T>& position, const Vector3<T>& target, const Vector3<T>& up) {
        return static_cast<Matrix4<T>>(glm::lookAt(
            reinterpret_cast<const glm::vec<3, T>&>(position),
            reinterpret_cast<const glm::vec<3, T>&>(target),
            reinterpret_cast<const glm::vec<3, T>&>(up)));
    }

    export template <typename T = f32> requires std::is_floating_point_v<T>
    constexpr Quaternion<T> LookAt(const Vector3<T>& direction, const Vector3<T>& up) {
        return Quaternion(glm::quatLookAt(
            reinterpret_cast<const glm::vec<3, T>&>(direction),
            reinterpret_cast<const glm::vec<3, T>&>(up)));
    }

	export template <typename T = f32> requires std::is_floating_point_v<T>
    constexpr Vector3<T> Direction(const Vector3<T>& eulerAngles) {
    	return Vector3 {
    		glm::cos(eulerAngles.y) * glm::cos(eulerAngles.x),
			glm::sin(eulerAngles.x),
			glm::sin(eulerAngles.y) * glm::cos(eulerAngles.x)
		}.Normalized();
    }
}