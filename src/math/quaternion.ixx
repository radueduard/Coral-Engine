//
// Created by radue on 4/16/2025.
//
export module math.quaternion;

import <glm/glm.hpp>;
import <glm/gtc/quaternion.hpp>;
import <glm/gtx/quaternion.hpp>;

import std;

import types;
import math.vector;
import math.matrix;

namespace Coral::Math {
    struct Quaternion : Vector4<f32> {
        constexpr Quaternion() : Vector4<f32> { 0.0f, 0.0f, 0.0f, 1.0f } {}
        constexpr Quaternion(const f32 w, const f32 x, const f32 y, const f32 z) : Vector4<f32> { x, y, z, w } {}

        constexpr explicit Quaternion(const Vector3<f32>& eulerAngles) {
            *this = glm::quat(glm::radians(glm::vec3(eulerAngles)));
        }

        constexpr Quaternion(glm::quat q) : Vector4<f32> { q.x, q.y, q.z, q.w } {}

        constexpr static Quaternion Identity() {
            return { 1.0f, 0.0f, 0.0f, 0.0f };
        }

        [[nodiscard]] Quaternion Normalized() const {
            return glm::normalize(reinterpret_cast<const glm::quat&>(*this));
        }

        constexpr void Normalize() {
            *this = Normalized();
        }

        // Converts the quaternion to Euler angles (in radians)
        [[nodiscard]] Vector3<f32> ToEulerAngles() const {
            return ToEulerAngles(*this);
        }

        // Converts a quaternion to Euler angles (in radians)
        static Vector3<f32> ToEulerAngles(const Quaternion& q) {
            const auto normalized = q.Normalized();
            const auto eulerAngles = glm::eulerAngles(glm::normalize(reinterpret_cast<const glm::quat&>(normalized)));
            return reinterpret_cast<const Vector3<f32>&>(eulerAngles);
        }

        static Quaternion FromEulerAngles(const Vector3<f32>& eulerAngles) {
            return Quaternion(eulerAngles);
        }

        static Quaternion FromAxisAngle(const Vector3<f32>& axis, const f32 angle) {
            const f32 halfAngle = angle * 0.5f;
            const f32 s = std::sin(halfAngle);
            return { std::cos(halfAngle), axis.x * s, axis.y * s, axis.z * s };
        }

        /*
         * Converts a quaternion to an axis-angle representation.
         * The angle is in radians, and the axis is normalized.
         *
         * @param q The quaternion to convert.
         * @return A quaternion representing the axis and angle. (x, y, z are the axis, w is the angle)
         */
        static Quaternion ToAxisAngle(const Quaternion& q) {
            const f32 angle = 2.0f * std::acos(q.w);
            const f32 s = std::sqrt(1.0f - q.w * q.w);
            if (s < 0.001f) {
                return { 1.0f, 0.0f, 0.0f, 0.0f };
            }
            return { q.x / s, q.y / s, q.z / s, angle };
        }

        operator glm::quat() const { return reinterpret_cast<const glm::quat&>(*this); }
        operator Matrix4<f32>() const {
            glm::mat4 m = glm::toMat4(reinterpret_cast<const glm::quat&>(*this));
            return reinterpret_cast<Matrix4<f32>&>(m);
        }
    };
}
