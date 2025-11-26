//
// Created by radue on 13/07/2025.
//

#pragma once

#include <algorithm>
#include <array>
#include <assimp/vector2.h>
#include <assimp/vector3.h>
#include <stdexcept>
#include <type_traits>

#include "data.h"
#include "utils/types.h"

#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

namespace Coral::Math {
    template<typename T, u8 N> requires (N > 1) && std::is_arithmetic_v<T>
    struct Vector : Data<T, N> {
        constexpr Vector() : Data<T, N>() {}

        explicit constexpr Vector(T v) : Data<T, N>(v) {}

        template<typename... Args> requires (sizeof...(Args) == N) && (std::is_same_v<T, Args> && ...)
        constexpr Vector(Args... args) : Data<T, N>({static_cast<T>(args)...}) {}

        template<typename... Args> requires (sizeof...(Args) < N) && (std::is_same_v<T, Args> && ...)
        constexpr Vector(Args... args) : Data<T, N>({static_cast<T>(args)..., static_cast<T>(0)}) {}

        template<u8 M> requires (M >= N)
        explicit constexpr Vector(const Vector<T, M>& other) {
            std::copy(other.data.begin(), other.data.begin() + N, this->data.data());
        }

        template<u8 M, typename... Args> requires (M < N) && (std::is_same_v<T, Args> && ...) && (sizeof...(Args) <= N - M)
        explicit constexpr Vector(const Vector<T, M>& other, Args... args) {
            std::copy(other.data.begin(), other.data.begin() + M, this->data.data());
            std::array<T, N - M> rest = { args... };
            std::copy(rest.begin(), rest.end(), this->data.begin() + M);
        }

    	template <typename Q> requires std::is_arithmetic_v<Q> && (!std::is_same_v<Q, T>)
		constexpr Vector(const Vector<Q, N>& other) {
        	for (u8 i = 0; i < N; ++i) {
				this->data[i] = static_cast<T>(other.data[i]);
			}
        }

        constexpr static Vector Zero() { return Vector(0); }

        // Equality and comparison operators

        constexpr bool operator==(const Vector& other) const {
            for (u8 i = 0; i < N; ++i) {
                if (this->data[i] != other.data[i]) {
                    return false;
                }
            }
            return true;
        }

        constexpr bool operator!=(const Vector& other) const {
            return !(*this == other);
        }

        // Arithmetic operations

        constexpr Vector operator+(const Vector& other) const {
            Vector result;
            for (u8 i = 0; i < N; ++i) {
                result.data[i] = this->data[i] + other.data[i];
            }
            return result;
        }

    	constexpr Vector operator+(T scalar) const {
        	Vector result;
        	for (u8 i = 0; i < N; ++i) {
        		result.data[i] = this->data[i] + scalar;
        	}
        	return result;
        }

        constexpr Vector operator-(const Vector& other) const {
            Vector result;
            for (u8 i = 0; i < N; ++i) {
                result.data[i] = this->data[i] - other.data[i];
            }
            return result;
        }

    	constexpr Vector operator-(T scalar) const {
			Vector result;
			for (u8 i = 0; i < N; ++i) {
				result.data[i] = this->data[i] - scalar;
			}
			return result;
		}

        constexpr Vector operator*(const Vector& other) const {
            Vector result;
            for (u8 i = 0; i < N; ++i) {
                result.data[i] = this->data[i] * other.data[i];
            }
            return result;
        }

    	constexpr Vector operator*(T scalar) const {
			Vector result;
			for (u8 i = 0; i < N; ++i) {
				result.data[i] = this->data[i] * scalar;
			}
			return result;
		}

        constexpr Vector operator/(const Vector& other) const {
            Vector result;
            for (u8 i = 0; i < N; ++i) {
                if (other.data[i] == 0) {
                    throw std::runtime_error("Division by zero");
                }
                result.data[i] = this->data[i] / other.data[i];
            }
            return result;
        }

    	constexpr Vector operator/(T scalar) const {
			if (scalar == 0) {
				throw std::runtime_error("Division by zero");
			}
			Vector result;
			for (u8 i = 0; i < N; ++i) {
				result.data[i] = this->data[i] / scalar;
			}
			return result;
		}

        constexpr Vector& operator+=(const Vector& other) {
            for (u8 i = 0; i < N; ++i) {
                this->data[i] += other.data[i];
            }
            return *this;
        }

        constexpr Vector& operator+=(T scalar) {
            for (u8 i = 0; i < N; ++i) {
                this->data[i] += scalar;
            }
            return *this;
        }

        constexpr Vector& operator-=(const Vector& other) {
            for (u8 i = 0; i < N; ++i) {
                this->data[i] -= other.data[i];
            }
            return *this;
        }

        constexpr Vector& operator-=(T scalar) {
            for (u8 i = 0; i < N; ++i) {
                this->data[i] -= scalar;
            }
            return *this;
        }

        constexpr Vector& operator*=(const Vector& other) {
            for (u8 i = 0; i < N; ++i) {
                this->data[i] *= other.data[i];
            }
            return *this;
        }

        constexpr Vector& operator*=(T scalar) {
            for (u8 i = 0; i < N; ++i) {
                this->data[i] *= scalar;
            }
            return *this;
        }

        constexpr Vector& operator/=(const Vector& other) {
            for (u8 i = 0; i < N; ++i) {
                if (other.data[i] == 0) {
                    throw std::runtime_error("Division by zero");
                }
                this->data[i] /= other.data[i];
            }
            return *this;
        }

        constexpr Vector& operator/=(T scalar) {
            if (scalar == 0) {
                throw std::runtime_error("Division by zero");
            }
            for (u8 i = 0; i < N; ++i) {
                this->data[i] /= scalar;
            }
            return *this;
        }

        constexpr Vector operator-() const {
            Vector result;
            for (u8 i = 0; i < N; ++i) {
                result.data[i] = -this->data[i];
            }
            return result;
        }

        constexpr T Length() const {
            return glm::length(glm::vec<N, T>(*this));
        }

        constexpr T Dot(const Vector& other) const {
            T sum = 0;
            for (u8 i = 0; i < N; ++i) {
                sum += this->data[i] * other.data[i];
            }
            return sum;
        }

        constexpr static T Dot(const Vector& a, const Vector& b) {
            return a.Dot(b);
        }

        constexpr Vector& Normalize() {
            T len = this->Length();
            if (len == 0) {
                throw std::runtime_error("Cannot normalize a zero vector");
            }
            for (u8 i = 0; i < N; ++i) {
                this->data[i] /= len;
            }
            return *this;
        }

        constexpr Vector Normalized() const {
            Vector result = *this;
            result.Normalize();
            return result;
        }

        constexpr Vector<T, 3> Cross(const Vector<T, 3>& other) const requires (N == 3) {
            return Vector(glm::cross(glm::vec3(*this), glm::vec3(other)));
        }

        constexpr static Vector Cross(const Vector<T, 3>& a, const Vector<T, 3>& b) requires (N == 3) {
            return a.Cross(b);
        }

        constexpr static Vector Min(const Vector& lhs, const Vector& rhs) {
            Vector result;
            for (u8 i = 0; i < N; ++i) {
                result.data[i] = std::min(lhs.data[i], rhs.data[i]);
            }
            return result;
        }

        constexpr static Vector Max(const Vector& lhs, const Vector& rhs) {
            Vector result;
            for (u8 i = 0; i < N; ++i) {
                result.data[i] = std::max(lhs.data[i], rhs.data[i]);
            }
            return result;
        }

        // Conversion operators and constructors

        constexpr explicit Vector(const glm::vec<N, T>& other) {
            for (int i = 0; i < N; ++i) {
                this->data[i] = other[i];
            }
        }

        constexpr explicit operator glm::vec<N, T>() const {
            glm::vec<N, T> result;
            std::copy(this->data.begin(), this->data.end(), &result[0]);
            return result;
        }

    	constexpr explicit operator ImVec2() const requires (N == 2) && std::is_same_v<T, f32> {
			return ImVec2(this->data[0], this->data[1]);
		}

    	constexpr explicit Vector(ImVec2 vec) requires (N == 2) && std::is_same_v<T, f32> {
			this->data[0] = vec.x;
			this->data[1] = vec.y;
		}

		constexpr explicit operator ImVec4() const requires (N == 4) && std::is_same_v<T, f32> {
			return ImVec4(this->data[0], this->data[1], this->data[2], this->data[3]);
		}

    	constexpr explicit Vector(ImVec4 vec) requires (N == 4) && std::is_same_v<T, f32> {
        	this->data[0] = vec.x;
        	this->data[1] = vec.y;
        	this->data[2] = vec.z;
        	this->data[3] = vec.w;
        }

    	constexpr explicit operator vk::Extent2D() const requires (N == 2) && std::is_same_v<T, u32> {
        	return vk::Extent2D(this->data[0], this->data[1]);
        }

    	constexpr explicit Vector(vk::Extent2D extent) requires (N == 2) && std::is_same_v<T, u32> {
			this->data[0] = extent.width;
			this->data[1] = extent.height;
		}

    	constexpr explicit operator vk::Offset2D() const requires (N == 2) && std::is_same_v<T, i32> {
			return vk::Offset2D(this->data[0], this->data[1]);
		}

		constexpr explicit Vector(vk::Offset2D offset) requires (N == 2) && std::is_same_v<T, i32> {
        	this->data[0] = offset.x;
        	this->data[1] = offset.y;
        }

    	constexpr explicit operator vk::Offset3D() const requires (N == 3) && std::is_same_v<T, i32> {
	        return vk::Offset3D(this->data[0], this->data[1], this->data[2]);
        }

    	constexpr explicit Vector(vk::Offset3D offset) requires (N == 3) && std::is_same_v<T, i32> {
			this->data[0] = offset.x;
			this->data[1] = offset.y;
			this->data[2] = offset.z;
		}

    	constexpr explicit operator vk::Extent3D() const requires (N == 3) && std::is_same_v<T, u32> {
        	return vk::Extent3D(this->data[0], this->data[1], this->data[2]);
        }

    	constexpr explicit Vector(vk::Extent3D extent) requires (N == 3) && std::is_same_v<T, u32> {
        	this->data[0] = extent.width;
        	this->data[1] = extent.height;
        	this->data[2] = extent.depth;
        }

    	constexpr explicit Vector(aiVector2D other) {
        	static_assert(N == 2, "Vector must be of size 2");
        	static_assert(std::is_same_v<T, f32>, "Vector must be of type f32");

        	this->data[0] = other.x;
        	this->data[1] = other.y;
        }

    	constexpr explicit Vector(aiVector3D other) {
        	static_assert(N == 3, "Vector must be of size 3");
        	static_assert(std::is_same_v<T, f32>, "Vector must be of type f32");

        	this->data[0] = other.x;
        	this->data[1] = other.y;
        	this->data[2] = other.z;
        }
    };

    template <typename T> requires std::is_arithmetic_v<T>
    using Vector2 = Vector<T, 2>;

    template <typename T> requires std::is_arithmetic_v<T>
    using Vector3 = Vector<T, 3>;

    template <typename T> requires std::is_arithmetic_v<T>
    using Vector4 = Vector<T, 4>;


    using Vector2f = Vector<f32, 2>;
    using Vector3f = Vector<f32, 3>;
    using Vector4f = Vector<f32, 4>;

    using Vector2i = Vector<i32, 2>;
    using Vector3i = Vector<i32, 3>;
    using Vector4i = Vector<i32, 4>;

    using Vector2u = Vector<u32, 2>;
    using Vector3u = Vector<u32, 3>;
    using Vector4u = Vector<u32, 4>;

    using Vector2d = Vector<f64, 2>;
    using Vector3d = Vector<f64, 3>;
    using Vector4d = Vector<f64, 4>;
}

template<typename T, Coral::u8 N>
struct std::formatter<Coral::Math::Vector<T, N>> : std::formatter<T> {
    template<typename FormatContext>
    auto format(const Coral::Math::Vector<T, N>& vec, FormatContext& ctx) const {
        auto out = ctx.out();
        out = std::format_to(out, "[");
        for (Coral::u8 i = 0; i < N; ++i) {
            out = std::formatter<T>::format(vec[i], ctx);
            if (i < N - 1) {
                out = std::format_to(out, ", ");
            }
        }
        out = std::format_to(out, "]");
        return out;
    }
};