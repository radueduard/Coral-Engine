//
// Created by radue on 14/07/2025.
//

export module math.frustum;
import math.vector;
import math.aabb;

import types;
import <glm/glm.hpp>;

namespace Coral::Math {
    export class Frustum {
    public:
        enum class Plane : u8 {
            Near,
            Far,
            Left,
            Right,
            Top,
            Bottom
          };

        struct FrustumPlane {
            Vector3<f32> normal {};
            f32 distance {};

            [[nodiscard]]
            bool IsOnPositiveSide(const Vector3<f32>& point) const {
                return Vector3<f32>::Dot(normal, point) + distance > 0.0f;
            }
        };

        void Update(const Vector3<f32>& position, const Vector3<f32>& direction, const Vector3<f32>& up, f32 fov, f32 aspectRatio, f32 nearPlane, f32 farPlane) {
            // Normalize input vectors
            Vector3<f32> normalizedDirection = direction.Normalized();
            Vector3<f32> normalizedUp = up.Normalized();
            Vector3<f32> right = Vector3<f32>::Cross(normalizedDirection, normalizedUp).Normalized();
            Vector3<f32> trueUp = Vector3<f32>::Cross(right, normalizedDirection).Normalized();

            // Cache values
            f32 halfVSide = farPlane * glm::tan(fov * 0.5f);
            f32 halfHSide = halfVSide * aspectRatio;
            Vector3<f32> frontMultFar = normalizedDirection * farPlane;
            Vector3<f32> frontMultNear = normalizedDirection * nearPlane;

            // Calculate corners
            auto nearCenter = position + frontMultNear;
            auto farCenter = position + frontMultFar;

            // Calculate the 8 corners of the frustum
            Vector3<f32> farTopLeft = farCenter + (trueUp * halfVSide) - (right * halfHSide);
            Vector3<f32> farTopRight = farCenter + (trueUp * halfVSide) + (right * halfHSide);
            Vector3<f32> farBottomLeft = farCenter - (trueUp * halfVSide) - (right * halfHSide);
            Vector3<f32> farBottomRight = farCenter - (trueUp * halfVSide) + (right * halfHSide);

            f32 nearHalfVSide = nearPlane * std::tanf(fov * 0.5f);
            f32 nearHalfHSide = nearHalfVSide * aspectRatio;
            Vector3<f32> nearTopLeft = nearCenter + (trueUp * nearHalfVSide) - (right * nearHalfHSide);
            Vector3<f32> nearTopRight = nearCenter + (trueUp * nearHalfVSide) + (right * nearHalfHSide);
            Vector3<f32> nearBottomLeft = nearCenter - (trueUp * nearHalfVSide) - (right * nearHalfHSide);
            Vector3<f32> nearBottomRight = nearCenter - (trueUp * nearHalfVSide) + (right * nearHalfHSide);

            // Calculate the 6 planes
            m_planes[static_cast<i32>(Plane::Near)].normal = -normalizedDirection;
            m_planes[static_cast<i32>(Plane::Near)].distance = -Vector3<f32>::Dot(-normalizedDirection, nearCenter);

            m_planes[static_cast<i32>(Plane::Far)].normal = normalizedDirection;
            m_planes[static_cast<i32>(Plane::Far)].distance = -Vector3<f32>::Dot(normalizedDirection, farCenter);

            Vector3<f32> leftNormal = Vector3<f32>::Cross(farTopLeft - nearTopLeft, nearBottomLeft - nearTopLeft).Normalized();
            m_planes[static_cast<i32>(Plane::Left)].normal = leftNormal;
            m_planes[static_cast<i32>(Plane::Left)].distance = -Vector3<f32>::Dot(leftNormal, nearTopLeft);

            Vector3<f32> rightNormal = Vector3<f32>::Cross(nearBottomRight - nearTopRight, farTopRight - nearTopRight).Normalized();
            m_planes[static_cast<i32>(Plane::Right)].normal = rightNormal;
            m_planes[static_cast<i32>(Plane::Right)].distance = -Vector3<f32>::Dot(rightNormal, nearTopRight);

            Vector3<f32> topNormal = Vector3<f32>::Cross(farTopRight - nearTopRight, nearTopLeft - nearTopRight).Normalized();
            m_planes[static_cast<i32>(Plane::Top)].normal = topNormal;
            m_planes[static_cast<i32>(Plane::Top)].distance = -Vector3<f32>::Dot(topNormal, nearTopRight);

            Vector3<f32> bottomNormal = Vector3<f32>::Cross(nearBottomLeft - nearBottomRight, farBottomRight - nearBottomRight).Normalized();
            m_planes[static_cast<i32>(Plane::Bottom)].normal = bottomNormal;
            m_planes[static_cast<i32>(Plane::Bottom)].distance = -Vector3<f32>::Dot(bottomNormal, nearBottomRight);
        }

        bool Contains(const Vector3<f32>& point) const {
            // Check if point is on positive side of all planes
            for (i32 i = 0; i < 6; ++i) {
                if (!m_planes[i].IsOnPositiveSide(point)) {
                    return false;
                }
            }
            return true;
        }

        [[nodiscard]]
        bool Intersects(const Vector3<f32>& point, const f32 radius) const {
            for (i32 i = 0; i < 6; ++i) {
                f32 distance = Vector3<f32>::Dot(m_planes[i].normal, point) + m_planes[i].distance;
                if (distance < -radius) {
                    return false;
                }
            }
            return true;
        }

        [[nodiscard]]
        bool Intersects(const AABB<f32> & aabb) const {
            // Get the AABB corners
            Vector3<f32> corners[8];
            corners[0] = aabb.min;
            corners[1] = { aabb.max.x, aabb.min.y, aabb.min.z };
            corners[2] = { aabb.min.x, aabb.max.y, aabb.min.z };
            corners[3] = { aabb.max.x, aabb.max.y, aabb.min.z };
            corners[4] = { aabb.min.x, aabb.min.y, aabb.max.z };
            corners[5] = { aabb.max.x, aabb.max.y, aabb.max.z };
            corners[6] = { aabb.min.x, aabb.max.y, aabb.max.z };
            corners[7] = aabb.max;

            // For each frustum plane
            for (i32 i = 0; i < 6; ++i) {
                bool allOutside = true;

                // Check all corners of AABB against this plane
                for (i32 j = 0; j < 8; ++j) {
                    if (m_planes[i].IsOnPositiveSide(corners[j])) {
                        allOutside = false;
                        break;
                    }
                }

                if (allOutside) {
                    return false; // All corners are outside this plane
                }
            }

            return true; // AABB intersects or is inside the frustum
        }

    private:
        FrustumPlane m_planes[6] = {};
    };
}