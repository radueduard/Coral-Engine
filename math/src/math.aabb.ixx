//
// Created by radue on 14/07/2025.
//

export module math.aabb;
export import math.vector;

import types;
import std;

namespace Coral::Math {
    export template<typename T> requires std::is_floating_point_v<T>
    struct  AABB {
        Vector3<T> min;
        Vector3<T> max;

        constexpr AABB() = default;
        constexpr explicit AABB(const Vector3<T>& min, const Vector3<T>& max) : min(min), max(max) {}


        constexpr void Grow(const Vector3<T>& point) {
            min = Vector3<T>::Min(min, point);
            max = Vector3<T>::Max(max, point);
        }

        constexpr void Grow(const AABB& aabb) {
            min = Vector3<T>::Min(min, aabb.min);
            max = Vector3<T>::Max(max, aabb.max);
        }

        constexpr static AABB Union(const AABB& a, const AABB& b) {
            return AABB(Vector3<T>::Min(a.min, b.min), Vector3<T>::Max(a.max, b.max));
        }
    };
}