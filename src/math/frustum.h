//
// Created by radue on 5/12/2025.
//

#pragma once

#include "aabb.h"
#include "vector.h"

namespace Coral::Math {
	class Frustum {
	public:
		enum class Plane {
			Near,
			Far,
			Left,
			Right,
			Top,
			Bottom,
			Count
		  };

		struct FrustumPlane {
			Vector3<f32> normal;
			float distance;

			bool IsOnPositiveSide(const Vector3<f32>& point) const {
				return Vector3<f32>::Dot(normal, point) + distance > 0.0f;
			}
		};

		Frustum() = default;
		~Frustum() = default;

		void Update(const Vector3<f32>& position, const Vector3<f32>& direction, const Vector3<f32>& up, f32 fov, f32 aspectRatio, f32 nearPlane, f32 farPlane) {
			// Normalize input vectors
			Vector3<f32> normalizedDirection = direction.Normalize();
			Vector3<f32> normalizedUp = up.Normalize();
			Vector3<f32> right = Vector3<f32>::Cross(normalizedDirection, normalizedUp).Normalize();
			Vector3<f32> trueUp = Vector3<f32>::Cross(right, normalizedDirection).Normalize();

			// Cache values
			f32 halfVSide = farPlane * tanf(fov * 0.5f);
			f32 halfHSide = halfVSide * aspectRatio;
			Vector3<f32> frontMultFar = normalizedDirection * farPlane;
			Vector3<f32> frontMultNear = normalizedDirection * nearPlane;

			// Calculate corners
			m_nearCenter = position + frontMultNear;
			m_farCenter = position + frontMultFar;

			// Calculate the 8 corners of the frustum
			Vector3<f32> farTopLeft = m_farCenter + (trueUp * halfVSide) - (right * halfHSide);
			Vector3<f32> farTopRight = m_farCenter + (trueUp * halfVSide) + (right * halfHSide);
			Vector3<f32> farBottomLeft = m_farCenter - (trueUp * halfVSide) - (right * halfHSide);
			Vector3<f32> farBottomRight = m_farCenter - (trueUp * halfVSide) + (right * halfHSide);

			f32 nearHalfVSide = nearPlane * std::tanf(fov * 0.5f);
			f32 nearHalfHSide = nearHalfVSide * aspectRatio;
			Vector3<f32> nearTopLeft = m_nearCenter + (trueUp * nearHalfVSide) - (right * nearHalfHSide);
			Vector3<f32> nearTopRight = m_nearCenter + (trueUp * nearHalfVSide) + (right * nearHalfHSide);
			Vector3<f32> nearBottomLeft = m_nearCenter - (trueUp * nearHalfVSide) - (right * nearHalfHSide);
			Vector3<f32> nearBottomRight = m_nearCenter - (trueUp * nearHalfVSide) + (right * nearHalfHSide);

			// Calculate the 6 planes
			m_planes[static_cast<i32>(Plane::Near)].normal = -normalizedDirection;
			m_planes[static_cast<i32>(Plane::Near)].distance = -Vector3<f32>::Dot(-normalizedDirection, m_nearCenter);

			m_planes[static_cast<i32>(Plane::Far)].normal = normalizedDirection;
			m_planes[static_cast<i32>(Plane::Far)].distance = -Vector3<f32>::Dot(normalizedDirection, m_farCenter);

			Vector3<f32> leftNormal = Vector3<f32>::Cross(farTopLeft - nearTopLeft, nearBottomLeft - nearTopLeft).Normalize();
			m_planes[static_cast<i32>(Plane::Left)].normal = leftNormal;
			m_planes[static_cast<i32>(Plane::Left)].distance = -Vector3<f32>::Dot(leftNormal, nearTopLeft);

			Vector3<f32> rightNormal = Vector3<f32>::Cross(nearBottomRight - nearTopRight, farTopRight - nearTopRight).Normalize();
			m_planes[static_cast<i32>(Plane::Right)].normal = rightNormal;
			m_planes[static_cast<i32>(Plane::Right)].distance = -Vector3<f32>::Dot(rightNormal, nearTopRight);

			Vector3<f32> topNormal = Vector3<f32>::Cross(farTopRight - nearTopRight, nearTopLeft - nearTopRight).Normalize();
			m_planes[static_cast<i32>(Plane::Top)].normal = topNormal;
			m_planes[static_cast<i32>(Plane::Top)].distance = -Vector3<f32>::Dot(topNormal, nearTopRight);

			Vector3<f32> bottomNormal = Vector3<f32>::Cross(nearBottomLeft - nearBottomRight, farBottomRight - nearBottomRight).Normalize();
			m_planes[static_cast<i32>(Plane::Bottom)].normal = bottomNormal;
			m_planes[static_cast<i32>(Plane::Bottom)].distance = -Vector3<f32>::Dot(bottomNormal, nearBottomRight);
		}

		bool Contains(const Vector3<f32>& point) const {
			// Check if point is on positive side of all planes
			for (i32 i = 0; i < static_cast<i32>(Plane::Count); ++i) {
				if (!m_planes[i].IsOnPositiveSide(point)) {
					return false;
				}
			}
			return true;
		}

		bool Intersects(const Vector3<f32>& point, const f32 radius) const {
			for (i32 i = 0; i < static_cast<int>(Plane::Count); ++i) {
				float distance = Vector3<f32>::Dot(m_planes[i].normal, point) + m_planes[i].distance;
				if (distance < -radius) {
					return false;
				}
			}
			return true;
		}

		bool Intersects(const AABB& aabb) const {
			// Get the AABB corners
			Vector3<f32> corners[8];
			corners[0] = aabb.Min();
			corners[1] = Vector3 { aabb.Max().x, aabb.Min().y, aabb.Min().z };
			corners[2] = Vector3 { aabb.Min().x, aabb.Max().y, aabb.Min().z };
			corners[3] = Vector3 { aabb.Max().x, aabb.Max().y, aabb.Min().z };
			corners[4] = Vector3 { aabb.Min().x, aabb.Min().y, aabb.Max().z };
			corners[5] = Vector3 { aabb.Max().x, aabb.Min().y, aabb.Max().z };
			corners[6] = Vector3 { aabb.Min().x, aabb.Max().y, aabb.Max().z };
			corners[7] = aabb.Max();

			// For each frustum plane
			for (i32 i = 0; i < static_cast<i32>(Plane::Count); ++i) {
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
		FrustumPlane m_planes[static_cast<i32>(Plane::Count)];
		Vector3<f32> m_nearCenter;
		Vector3<f32> m_farCenter;
	};
}