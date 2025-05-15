//
// Created by radue on 4/16/2025.
//
#pragma once

import <glm/ext/scalar_constants.hpp>;
import <glm/gtc/constants.hpp>;

#include "vector.h"


namespace Coral::Math {
    template<typename T> requires std::is_floating_point_v<T>
    constexpr T Pi() {
        return glm::pi<T>();
    }

    template<typename T> requires std::is_floating_point_v<T>
    constexpr T Radians(T degrees) {
        return degrees * Pi<T>() / static_cast<T>(180);
    }

    template<typename T, i32 N> requires std::is_floating_point_v<T>
    constexpr Vector<T, N> Radians(const Vector<T, N>& degrees) {
        return degrees * Pi<T>() / static_cast<T>(180);
    }

    template<typename T> requires std::is_floating_point_v<T>
    constexpr T Degrees(T radians) {
        return radians * static_cast<T>(180) / Pi<T>();
    }

    template<typename T, i32 N> requires std::is_floating_point_v<T>
    constexpr Vector<T, N> Degrees(const Vector<T, N>& radians) {
        return radians * static_cast<T>(180) / Pi<T>();
    }

}