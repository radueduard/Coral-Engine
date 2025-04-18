//
// Created by radue on 4/15/2025.
//

export module math.vector;

import <vulkan/vulkan.hpp>;
import <glm/glm.hpp>;
import <imgui.h>;

import std;
import types;

namespace Coral::Math {
	template<typename T, u32 N> requires std::is_arithmetic_v<T> && (N > 0)
	struct Data {
		T data[N];
	};

	template<typename T>
	struct Data<T, 1> {
		union {
			T data[1];
			struct { T x; };
			struct { T r; };
		};
	};

	template<typename T>
	struct Data<T, 2> {
		union {
			T data[2];
			struct { T x, y; };
			struct { T r, g; };
			struct { T u, v; };
		};
	};

	template<typename T>
	struct Data<T, 3> {
		union {
			T data[3];
			struct { T x, y, z; };
			struct { T r, g, b; };
		};
	};

	template<typename T>
	struct Data<T, 4> {
		union {
			T data[4];
			struct { T x, y, z, w; };
			struct { T r, g, b, a; };
		};
	};


	export template <typename T, u32 N> requires std::is_arithmetic_v<T> && (N > 0)
	struct Vector : Data<T, N> {
		constexpr explicit Vector() : Data<T, N>() {}

		constexpr explicit Vector(const T& scalar) {
			std::fill(this->data, this->data + N, scalar);
		}

		constexpr Vector(const std::initializer_list<T>& list) {
			std::copy(list.begin(), list.end(), this->data);
			std::fill(this->data + list.size(), this->data + N, static_cast<T>(0));
		}

		template <u32 M, typename... Args> requires (M > 0)
		constexpr explicit Vector(const Vector<T, M>& other, Args... args) {
			constexpr auto vectorSize = std::min(N, M);
			std::copy(other.data, other.data + vectorSize, this->data);
			if constexpr (vectorSize >= N) return;
			auto offsetData = this->data + M;
			for (const T& arg : { static_cast<T>(args)... }) {
				*offsetData = arg;
				++offsetData;
			}
			std::fill(offsetData, this->data + N, 0);
		}

		constexpr Vector(const Vector& other) {
			std::copy(other.data, other.data + N, this->data);
		}

		constexpr Vector(Vector&& other) noexcept {
			std::move(other.data, other.data + N, this->data);
		}

		constexpr Vector& operator=(const Vector& other) {
			if (this != &other) {
				std::copy(other.data, other.data + N, this->data);
			}
			return *this;
		}

		constexpr Vector& operator=(Vector&& other) noexcept {
			if (this != &other) {
				std::move(other.data, other.data + N, this->data);
			}
			return *this;
		}

		constexpr T& operator[](int index) {
			if (index < 0 || index >= N) {
				throw std::out_of_range("Index out of range");
			}
			return this->data[index];
		}

		constexpr const T& operator[](int index) const {
			if (index < 0 || index >= N) {
				throw std::out_of_range("Index out of range");
			}
			return this->data[index];
		}

		Vector operator+(const Vector& other) const {
			Vector result;
			for (int i = 0; i < N; ++i) {
				result.data[i] = this->data[i] + other.data[i];
			}
			return result;
		}

		Vector operator-(const Vector& other) const {
			Vector result;
			for (int i = 0; i < N; ++i) {
				result.data[i] = this->data[i] - other.data[i];
			}
			return result;
		}

		Vector operator-() const {
			Vector result;
			for (int i = 0; i < N; ++i) {
				result.data[i] = -this->data[i];
			}
			return result;
		}

		Vector operator*(const Vector& other) const {
			Vector result;
			for (int i = 0; i < N; ++i) {
				result.data[i] = this->data[i] * other.data[i];
			}
			return result;
		}

		Vector operator/(const Vector& other) const {
			Vector result;
			for (int i = 0; i < N; ++i) {
				if (other.data[i] == 0) {
					throw std::runtime_error("Division by zero");
				}
				result.data[i] = this->data[i] / other.data[i];
			}
			return result;
		}

		Vector operator+(const T& scalar) const {
			Vector result;
			for (int i = 0; i < N; ++i) {
				result.data[i] = this->data[i] + scalar;
			}
			return result;
		}

		Vector operator-(const T& scalar) const {
			Vector result;
			for (int i = 0; i < N; ++i) {
				result.data[i] = this->data[i] - scalar;
			}
			return result;
		}

		Vector operator*(const T& scalar) const {
			Vector result;
			for (int i = 0; i < N; ++i) {
				result.data[i] = this->data[i] * scalar;
			}
			return result;
		}

		Vector operator/(const T& scalar) const {
			if (scalar == 0) {
				throw std::runtime_error("Division by zero");
			}
			Vector result;
			for (int i = 0; i < N; ++i) {
				result.data[i] = this->data[i] / scalar;
			}
			return result;
		}

		constexpr bool operator==(const Vector& other) const {
			for (int i = 0; i < N; ++i) {
				if (this->data[i] != other.data[i]) {
					return false;
				}
			}
			return true;
		}

		constexpr bool operator!=(const Vector& other) const {
			return !(*this == other);
		}

		template <typename U, u32 M> requires std::is_arithmetic_v<U> && (M > 0)
		operator Vector<U, M>() const {
			Vector<U, M> result;
			for (int i = 0; i < std::min(N, M); ++i) {
				result.data[i] = static_cast<U>(this->data[i]);
			}
			for (int i = N; i < M; ++i) {
				result.data[i] = 0;
			}
			return result;
		}

		constexpr T Length() const {
			T length = 0;
			for (int i = 0; i < N; ++i) {
				length += this->data[i] * this->data[i];
			}
			return std::sqrt(length);
		}

		constexpr T Dot(const Vector& other) const {
			T dot = 0;
			for (int i = 0; i < N; ++i) {
				dot += this->data[i] * other.data[i];
			}
			return dot;
		}

		constexpr Vector Normalize() const {
			T length = Length();
			if (length == 0) {
				throw std::runtime_error("Cannot normalize a zero vector");
			}
			Vector result;
			for (int i = 0; i < N; ++i) {
				result.data[i] = this->data[i] / length;
			}
			return result;
		}

		constexpr static T Dot(const Vector& a, const Vector& b) {
			return a.Dot(b);
		}

		constexpr T SquaredNorm() const {
			T squaredNorm = 0;
			for (int i = 0; i < N; ++i) {
				squaredNorm += this->data[i] * this->data[i];
			}
			return squaredNorm;
		}

		constexpr static Vector Lerp(const Vector& a, const Vector& b, const T& t) {
			return a * (1 - t) + b * t;
		}

		constexpr static Vector Min(const Vector& a, const Vector& b) {
			Vector result;
			for (int i = 0; i < N; ++i) {
				result.data[i] = std::min(a.data[i], b.data[i]);
			}
			return result;
		}

		constexpr static Vector Max(const Vector& a, const Vector& b) {
			Vector result;
			for (int i = 0; i < N; ++i) {
				result.data[i] = std::max(a.data[i], b.data[i]);
			}
			return result;
		}

		constexpr operator std::array<T, N>() const {
			std::array<T, N> result;
			std::copy(this->data, this->data + N, result.data());
			return result;
		}

		constexpr operator glm::vec<N, T>() const {
			glm::vec<N, T> result;
			std::copy(this->data, &this->data[N], &result[0]);
			return result;
		}

		template <int M = N> requires (M == 2)
		constexpr Vector(const ImVec2& other) {
			this->data[0] = other.x;
			this->data[1] = other.y;
		}

		template <int M = N> requires (M == 2)
		constexpr operator ImVec2() const {
			return ImVec2(this->data[0], this->data[1]);
		}

		template <int M = N> requires (M == 2)
		constexpr operator vk::Extent2D() {
			return reinterpret_cast<vk::Extent2D&>(this->data);
		}

		template <int M = N> requires (M == 2)
		constexpr Vector(const vk::Extent2D& other) {
			this->data[0] = static_cast<T>(other.width);
			this->data[1] = static_cast<T>(other.height);
		}

		template <int M = N> requires (M == 3)
		constexpr operator vk::Extent3D() const {
			return reinterpret_cast<vk::Extent3D&>(this->data);
		}

		template <int M = N> requires (M == 3)
		constexpr Vector(const vk::Extent3D& other) {
			this->data[0] = static_cast<T>(other.width);
			this->data[1] = static_cast<T>(other.height);
			this->data[2] = static_cast<T>(other.depth);
		}

		template <int M = N> requires (M == 4)
		constexpr Vector(const ImVec4& other) {
			this->data[0] = other.x;
			this->data[1] = other.y;
			this->data[2] = other.z;
			this->data[3] = other.w;
		}

		template <int M = N> requires (M == 4)
		constexpr operator ImVec2() const {
			return ImVec4(this->data[0], this->data[1], this->data[2], this->data[3]);
		}

		constexpr static Vector Zero() { return Vector(0); }
		constexpr static Vector One() { return Vector(1); }
	};

	export template <typename T, u32 N> requires std::is_arithmetic_v<T> && (N > 0)
	std::ostream& operator<<(std::ostream& os, const Vector<T, N>& vec) {
		os << "(";
		for (int i = 0; i < N; ++i) {
			os << vec[i];
			if (i < N - 1) {
				os << ", ";
			}
		}
		os << ")";
		return os;
	}

	export template <typename T> requires std::is_arithmetic_v<T>
	using Vector2 = Vector<T, 2>;

	export template <typename T> requires std::is_arithmetic_v<T>
	using Vector3 = Vector<T, 3>;

	export template <typename T> requires std::is_arithmetic_v<T>
	using Vector4 = Vector<T, 4>;

}