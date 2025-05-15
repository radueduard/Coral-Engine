//
// Created by radue on 4/16/2025.
//
#pragma once

#include <imgui.h>
#include <imgui_internal.h>

#include "vector.h"

namespace Coral::Math {
    struct Rect {
        Vector2<f32> min;
        Vector2<f32> max;

        constexpr Rect() : min(Vector2<f32>::Zero()), max(Vector2<f32>::Zero()) {}
        constexpr Rect(const Vector2<f32>& min, const Vector2<f32>& max) : min(min), max(max) {}
        constexpr Rect(const ImRect& rect) : min(rect.Min), max(rect.Max) {}

        constexpr void GrowToInclude(const Rect& other) {
            min = Vector2<f32>::Min(min, other.min);
            max = Vector2<f32>::Max(max, other.max);
        }

        constexpr void GrowToInclude(const Vector2<float>& point) {
            min = Vector2<f32>::Min(min, point);
            max = Vector2<f32>::Max(max, point);
        }

        constexpr operator ImRect() const {
            return { static_cast<ImVec2>(min), static_cast<ImVec2>(max) };
        }

        constexpr static Rect Zero() {
            return { Vector2<f32>::Zero(), Vector2<f32>::Zero() };
        }
    };

}
