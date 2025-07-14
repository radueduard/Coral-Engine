//
// Created by radue on 14/07/2025.
//
#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

export module math.quaternion;
import math.vector;
import math.matrix;

import types;
import std;

namespace Coral::Math {
    export template <typename T> requires std::is_floating_point_v<T>
    struct Quaternion {
        T x, y, z, w;

        constexpr Quaternion() : x(0), y(0), z(0), w(1) {}
        constexpr Quaternion(const T w, const T x, const T y, const T z) : x(x), y(y), z(z), w(w) {}
        constexpr Quaternion(const T w, const Vector3<T>& v) : x(v.x), y(v.y), z(v.z), w(w) {}

        [[nodiscard]]
        static constexpr Quaternion Identity() {
            return { static_cast<T>(1), static_cast<T>(0), static_cast<T>(0), static_cast<T>(0) };
        }

        /**
         *
         * @param eulerAngles Euler angles in radians (pitch, yaw, roll)
         */
        constexpr explicit Quaternion(const Vector3<T>& eulerAngles) : Quaternion(glm::qua<T>(glm::vec<3, T>(eulerAngles))) {}

        [[nodiscard]] Quaternion Cross(const Quaternion& other) const {
            return Cross(*this, other);
        }

    	static Quaternion Cross(const Quaternion& lhs, const Quaternion& rhs) {
        	return Quaternion(glm::cross(reinterpret_cast<const glm::qua<T>&>(lhs), reinterpret_cast<const glm::qua<T>&>(rhs)));
        }

        [[nodiscard]] Quaternion Normalized() const {
            return Quaternion(glm::normalize(reinterpret_cast<const glm::qua<T>&>(*this)));
        }

        void Normalize() {
            *this = Normalized();
        }

    	Vector3<T> operator*(const Vector3<T>& v) const {
			return Vector3(reinterpret_cast<const glm::qua<T>&>(*this) * reinterpret_cast<const glm::vec<3, T>&>(v));
		}

		Quaternion operator*(const Quaternion& other) const {
			return Cross(*this, other);
		}

		Quaternion& operator*=(const Quaternion& other) {
			*this = Cross(*this, other);
			return *this;
		}

        [[nodiscard]] Vector3<T> ToEulerAngles() const {
            return ToEulerAngles(*this);
        }

        // Converts a quaternion to Euler angles (in radians)
        static Vector3<f32> ToEulerAngles(const Quaternion& q) {
            return Vector3(glm::eulerAngles(reinterpret_cast<const glm::qua<T>&>(q)));
        }

		/**
		 *
		 * @param eulerAngles euler angles in radians (x, y, z)
		 * @return Quaternion representing the rotation
		 */
		static Quaternion FromEulerAngles(const Vector3<T>& eulerAngles) {
            return Quaternion(eulerAngles);
        }

        static Quaternion FromAxisAngle(const T angle, const Vector3<T>& axis) {
            return Quaternion(glm::angleAxis(angle, reinterpret_cast<const glm::vec<3, T>&>(axis)));
        }

        /*
         * Converts a quaternion to an axis-angle representation.
         * The angle is in radians, and the axis is normalized.
         *
         * @param q The quaternion to convert.
         * @return A quaternion representing the axis and angle. (x, y, z are the axis, w is the angle)
         */
        static Quaternion ToAxisAngle(const Quaternion& q) {
            const auto angle = glm::angle(reinterpret_cast<const glm::qua<T>&>(q));
			const auto axis = glm::axis(reinterpret_cast<const glm::qua<T>&>(q));
			return Quaternion(angle, Vector3(axis));
        }

        // Conversion operators and constructors

        constexpr explicit Quaternion(const glm::qua<T> q) : x(q.x), y(q.y), z(q.z), w(q.w) {}
        explicit constexpr operator glm::qua<T>() const { return reinterpret_cast<const glm::qua<T>&>(*this); }

        operator Matrix4<T>() const {
            glm::mat<4, 4, T> m = glm::toMat4(reinterpret_cast<const glm::qua<T>&>(*this));
            return reinterpret_cast<Matrix4<T>&>(m);
        }

        Matrix4<T> ToMatrix() const {
            return static_cast<Matrix4<T>>(*this);
        }
    };
}
