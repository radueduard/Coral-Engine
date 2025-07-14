//
// Created by radue on 14/07/2025.
//

export module math.constants;
import math.vector;

import types;
import std;

import <glm/glm.hpp>;
import <glm/ext/scalar_constants.hpp>;
import <glm/gtc/constants.hpp>;

export namespace Coral::Math {
    template<typename T> requires std::is_floating_point_v<T>
    constexpr T Pi() {
        return glm::pi<T>();
    }

    template<typename T> requires std::is_floating_point_v<T>
    constexpr T Radians(T degrees) {
        return glm::radians(degrees);
    }

    template<typename T, i32 N> requires std::is_floating_point_v<T>
    constexpr Vector<T, N> Radians(const Vector<T, N>& degrees) {
        return Vector<T, N>(glm::radians(reinterpret_cast<const glm::vec<N, T>&>(degrees)));
    }

    template<typename T> requires std::is_floating_point_v<T>
    constexpr T Degrees(T radians) {
        return glm::degrees(radians);
    }

    template<typename T, i32 N> requires std::is_floating_point_v<T>
    constexpr Vector<T, N> Degrees(const Vector<T, N>& radians) {
        return Vector<T, N>(glm::degrees(reinterpret_cast<const glm::vec<N, T>&>(radians)));
    }

    template<typename T> requires std::is_floating_point_v<T>
    constexpr T Epsilon() {
        return glm::epsilon<T>();
    }
}