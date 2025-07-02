//
// Created by radue on 4/16/2025.
//
#pragma once


// #define GLM_FORCE_QUAT_DATA_WXYZ
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "vector.h"
#include "matrix.h"

namespace Coral::Math {
    struct Quaternion {

#ifdef GLM_FORCE_QUAT_DATA_WXYZ
		f32 w, x, y, z;
#else
    	f32 x, y, z, w;
#endif

        constexpr Quaternion() : w(1.0f), x(0.0f), y(0.0f), z(0.0f) {}
        constexpr Quaternion(const f32 w, const f32 x, const f32 y, const f32 z)
			: w(w), x(x), y(y), z(z) {}
		constexpr Quaternion(const f32 w, const Vector3<f32>& v)
			:w(w), x(v.x), y(v.y), z(v.z) {}

		/**
		 *
		 * @param eulerAngles Euler angles in radians (x, y, z)
		 */
		constexpr explicit Quaternion(const Vector3<f32>& eulerAngles) {
            *this = Quaternion(glm::quat(glm::vec3(eulerAngles)));
        }

        constexpr explicit Quaternion(const glm::quat q)
			: w(q.w), x(q.x), y(q.y), z(q.z) {}

        Quaternion(const Vector3<f32>& axis, const f32 angle) {
            *this = FromAxisAngle(angle, axis);
        }

        constexpr static Quaternion Identity() {
            return { 1.0f, 0.0f, 0.0f, 0.0f };
        }

        [[nodiscard]] Quaternion Cross(const Quaternion& other) const {
            return Cross(*this, other);
        }

    	static Quaternion Cross(const Quaternion& lhs, const Quaternion& rhs) {
        	return Quaternion(glm::cross(reinterpret_cast<const glm::quat&>(lhs), reinterpret_cast<const glm::quat&>(rhs)));
        }

        [[nodiscard]] Quaternion Normalized() const {
            return Quaternion(glm::normalize(reinterpret_cast<const glm::quat&>(*this)));
        }

        void Normalize() {
            *this = Normalized();
        }

    	Vector3<f32> operator*(const Vector3<f32>& v) const {
			return Vector3(reinterpret_cast<const glm::quat&>(*this) * reinterpret_cast<const glm::vec3&>(v));
		}

		Quaternion operator*(const Quaternion& other) const {
			return Cross(*this, other);
		}

		Quaternion& operator*=(const Quaternion& other) {
			*this = Cross(*this, other);
			return *this;
		}

        [[nodiscard]] Vector3<f32> ToEulerAngles() const {
            return ToEulerAngles(*this);
        }

        // Converts a quaternion to Euler angles (in radians)
        static Vector3<f32> ToEulerAngles(const Quaternion& q) {
            return Vector3(glm::eulerAngles(reinterpret_cast<const glm::quat&>(q)));
        }

		/**
		 *
		 * @param eulerAngles euler angles in radians (x, y, z)
		 * @return Quaternion representing the rotation
		 */
		static Quaternion FromEulerAngles(const Vector3<f32>& eulerAngles) {
            return Quaternion(eulerAngles);
        }

        static Quaternion FromAxisAngle(const f32 angle, const Vector3<f32>& axis) {
            return Quaternion(glm::angleAxis(angle, reinterpret_cast<const glm::vec3&>(axis)));
        }

        /*
         * Converts a quaternion to an axis-angle representation.
         * The angle is in radians, and the axis is normalized.
         *
         * @param q The quaternion to convert.
         * @return A quaternion representing the axis and angle. (x, y, z are the axis, w is the angle)
         */
        static Quaternion ToAxisAngle(const Quaternion& q) {
            const auto angle = glm::angle(reinterpret_cast<const glm::quat&>(q));
			const auto axis = glm::axis(reinterpret_cast<const glm::quat&>(q));
			return Quaternion(angle, Vector3 { axis.x, axis.y, axis.z });
        }

        operator glm::quat() const { return reinterpret_cast<const glm::quat&>(*this); }

        operator Matrix4<f32>() const {
            glm::mat4 m = glm::toMat4(reinterpret_cast<const glm::quat&>(*this));
            return reinterpret_cast<Matrix4<f32>&>(m);
        }

        Matrix4<f32> ToMatrix() const {
            return static_cast<Matrix4<f32>>(*this);
        }
    };
}
