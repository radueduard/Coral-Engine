//
// Created by radue on 14/07/2025.
//

export module math.rect;
import math.vector;

import types;
import std;

namespace Coral::Math {
    export template<typename T> requires std::is_floating_point_v<T>
    struct Rect {
        Vector2<T> min = Vector2<T>::Zero();
        Vector2<T> max = Vector2<T>::Zero();

        constexpr Rect() = default;
        constexpr Rect(const Vector2<T>& min, const Vector2<T>& max) : min(min), max(max) {}

        constexpr void GrowToInclude(const Rect& other) {
            min = Vector2<T>::Min(min, other.min);
            max = Vector2<T>::Max(max, other.max);
        }

        constexpr void GrowToInclude(const Vector2<float>& point) {
            min = Vector2<T>::Min(min, point);
            max = Vector2<T>::Max(max, point);
        }

        constexpr static Rect Zero() {
            return {};
        }
    };
}
