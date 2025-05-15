//
// Created by radue on 5/12/2025.
//

#pragma once

#include "math/vector.h"

namespace Coral::Math {
	class AABB {
	public:
		AABB() = default;
		~AABB() = default;

		explicit AABB(const Vector3<f32>& min, const Vector3<f32>& max)
			: m_min(min), m_max(max) {}

		[[nodiscard]] const Vector3<f32>& Min() const { return m_min; }
		[[nodiscard]] const Vector3<f32>& Max() const { return m_max; }

		void Grow(const Vector3<f32>& point) {
			m_min = Vector3<f32>::Min(m_min, point);
			m_max = Vector3<f32>::Max(m_max, point);
		}

		void Grow(const AABB& aabb) {
			m_min = Vector3<f32>::Min(m_min, aabb.m_min);
			m_max = Vector3<f32>::Max(m_max, aabb.m_max);
		}

		static AABB Union(const AABB& a, const AABB& b) {
			return AABB(Vector3<f32>::Min(a.m_min, b.m_min), Vector3<f32>::Max(a.m_max, b.m_max));
		}

	private:
		Vector3<f32> m_min;
		Vector3<f32> m_max;
	};
}