//
// Created by radue on 13/07/2025.
//

#pragma once

#include <utils/types.h>
#include <type_traits>

namespace Coral::Math {
    template<typename T, u8 N> requires std::is_arithmetic_v<T> && (N > 1)
    struct Data {
        std::array<T, N> data { static_cast<T>(0) };

        constexpr Data() = default;
        explicit constexpr Data(T v) { std::fill(data.begin(), data.end(), v); }

        constexpr explicit Data(std::array<T, N> init) {
            std::copy(init.begin(), init.end(), data.data());
        }

        constexpr T& operator[](u8 index) {
            return data[index];
        }

        constexpr const T& operator[](u8 index) const {
            return data[index];
        }

        [[nodiscard]]
        static constexpr u8 size() {
            return N;
        }
    };

    template<typename T> requires std::is_arithmetic_v<T>
    struct Data<T, 2> {
        union {
            std::array<T, 2> data { static_cast<T>(0) };
            struct { T x, y; };
            struct { T r, g; };
            struct { T s, t; };
        	struct { T width, height; };
        };

        constexpr Data() = default;
        constexpr explicit Data(const T v) : data { v, v } {}
        constexpr explicit Data(std::array<T, 2> init) {
            std::copy(init.begin(), init.end(), data.data());
        }

        constexpr T& operator[](const u8 index) {
            return data[index];
        }

        constexpr const T& operator[](const u8 index) const {
            return data[index];
        }

        [[nodiscard]]
        static constexpr u8 size() {
            return 2;
        }
    };

    template<typename T> requires std::is_arithmetic_v<T>
    struct Data<T, 3> {
        union {
            std::array<T, 3> data { static_cast<T>(0) };
            struct { T x, y, z; };
            struct { T r, g, b; };
            struct { T s, t, p; };
        	struct { T width, height, depth; };
        };

        constexpr Data() = default;
        constexpr explicit Data(const T v) : data { v, v, v } {}
        constexpr explicit Data(std::array<T, 3> init) {
            std::copy(init.begin(), init.end(), data.data());
        }

        constexpr T& operator[](u8 index) {
            return data[index];
        }

        constexpr const T& operator[](u8 index) const {
            return data[index];
        }

        [[nodiscard]]
        static constexpr u8 size() {
            return 3;
        }
    };

    template<typename T> requires std::is_arithmetic_v<T>
    struct Data<T, 4> {
        union {
            std::array<T, 4> data { static_cast<T>(0) };
            struct { T x, y, z, w; };
            struct { T r, g, b, a; };
            struct { T s, t, p, q; };
        };

        constexpr Data() = default;
        constexpr explicit Data(const T v) : data { v, v, v, v } {}
        constexpr Data(std::array<T, 4> init) {
            std::copy(init.begin(), init.end(), data.data());
        }

        constexpr T& operator[](u8 index) {
            return data[index];
        }

        constexpr const T& operator[](u8 index) const {
            return data[index];
        }

        [[nodiscard]]
        static constexpr u8 size() {
            return 4;
        }
    };
}