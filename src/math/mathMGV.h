//
// Created by radue on 2/20/2025.
//


#pragma once

#include <algorithm>
#include <glm/glm.hpp>
#include <imgui.h>
#include <imgui_internal.h>
#include <vulkan/vulkan.hpp>

namespace vk {
	struct Extent2D;
}

namespace Math {
	template <typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
	struct Vector2 {
		T x, y;

		Vector2() : x(0), y(0) {}
		Vector2(T scalar) : x(scalar), y(scalar) {}
		Vector2(T x, T y) : x(x), y(y) {}
		Vector2(const ImVec2& vec) : x(vec.x), y(vec.y) {}
		Vector2(const glm::vec2& vec) : x(vec.x), y(vec.y) {}
		Vector2(const vk::Extent2D& extent) : x(extent.width), y(extent.height) {}

		Vector2 operator+(const Vector2& other) const {
			return { x + other.x, y + other.y };
		}

		Vector2 operator-(const Vector2& other) const {
			return { x - other.x, y - other.y };
		}

		Vector2 operator*(const T scalar) const {
			return { x * scalar, y * scalar };
		}

		Vector2 operator/(const T scalar) const {
			return { x / scalar, y / scalar };
		}

		Vector2& operator+=(const Vector2& other) {
			x += other.x;
			y += other.y;
			return *this;
		}

		Vector2& operator-=(const Vector2& other) {
			x -= other.x;
			y -= other.y;
			return *this;
		}

		Vector2& operator*=(const T scalar) {
			x *= scalar;
			y *= scalar;
			return *this;
		}

		Vector2& operator/=(const T scalar) {
			x /= scalar;
			y /= scalar;
			return *this;
		}

		bool operator==(const Vector2& other) const {
			return x == other.x && y == other.y;
		}

		bool operator!=(const Vector2& other) const {
			return !(*this == other);
		}

		T Dot(const Vector2& other) const {
			return x * other.x + y * other.y;
		}

		T Length() const {
			return std::sqrt(x * x + y * y);
		}

		Vector2 Normalize() const {
			return *this / Length();
		}

		Vector2 Cross(Vector2 other) const {
			return { x * other.y - y * other.x, y * other.x - x * other.y };
		}

		static Vector2 Lerp(const Vector2& a, const Vector2& b, const T t) {
			return a + (b - a) * t;
		}

		static Vector2 Zero() {
			return { 0, 0 };
		}

		static Vector2 One() {
			return { 1, 1 };
		}

		static Vector2 Up() {
			return { 0, 1 };
		}

		static Vector2 Down() {
			return { 0, -1 };
		}

		operator glm::vec2() const {
			return { x, y };
		}

		operator ImVec2() const {
			return { x, y };
		}

		operator vk::Extent2D() const {
			return { static_cast<uint32_t>(x), static_cast<uint32_t>(y) };
		}

		template <typename U>
		operator Vector2<U>() const {
			return { static_cast<U>(x), static_cast<U>(y) };
		}

	};

	struct Rect {
		Vector2<float> min;
		Vector2<float> max;

		Rect() : min(Vector2<float>::Zero()), max(Vector2<float>::Zero()) {}
		Rect(const Vector2<float>& min, const Vector2<float>& max) : min(min), max(max) {}
		Rect(const ImRect& rect) : min(rect.Min), max(rect.Max) {}

		void GrowToInclude(const Rect& other) {
			min.x = std::min(min.x, other.min.x);
			min.y = std::min(min.y, other.min.y);
			max.x = std::max(max.x, other.max.x);
			max.y = std::max(max.y, other.max.y);
		}

		void GrowToInclude(const Vector2<float>& point) {
			min.x = std::min(min.x, point.x);
			min.y = std::min(min.y, point.y);
			max.x = std::max(max.x, point.x);
			max.y = std::max(max.y, point.y);
		}

		operator ImRect() const {
			return { min, max };
		}

		static Rect Zero() {
			return { Vector2<float>::Zero(), Vector2<float>::Zero() };
		}
	};

	struct Color {
		float r, g, b, a;

		Color() : r(1.f), g(1.f), b(1.f), a(1.f) {}
		Color(const float r, const float g, const float b, const float a = 1.f) : r(r), g(g), b(b), a(a) {}

		operator ImVec4() const {
			return { r, g, b, a };
		}

		operator glm::vec4() const {
			return { r, g, b, a };
		}

		operator ImColor() const {
			return { r, g, b, a };
		}

		static Color White() { return { 1.f, 1.f, 1.f, 1.f }; }
		static Color Black() { return { 0.f, 0.f, 0.f, 1.f }; }
		static Color Red() { return { 1.f, 0.f, 0.f, 1.f }; }
		static Color Green() { return { 0.f, 1.f, 0.f, 1.f }; }
		static Color Blue() { return { 0.f, 0.f, 1.f, 1.f }; }
		static Color Yellow() { return { 1.f, 1.f, 0.f, 1.f }; }
		static Color Cyan() { return { 0.f, 1.f, 1.f, 1.f }; }
		static Color Magenta() { return { 1.f, 0.f, 1.f, 1.f }; }

		template <float value>
		static Color Grey() { return { value, value, value, 1.f }; }

	};

	template <typename T>
	Vector2<T> Min(const Vector2<T>& a, const Vector2<T>& b) {
		return { std::min(a.x, b.x), std::min(a.y, b.y) };
	}

	template <typename T>
	Vector2<T> Max(const Vector2<T>& a, const Vector2<T>& b) {
		return { std::max(a.x, b.x), std::max(a.y, b.y) };
	}

	template <typename T>
	ImGuiDataType ImGuiDataType() {
		if (typeid(T) == typeid(int8_t)) return ImGuiDataType_S8;
		if (typeid(T) == typeid(uint8_t)) return ImGuiDataType_U8;
		if (typeid(T) == typeid(int16_t)) return ImGuiDataType_S16;
		if (typeid(T) == typeid(uint16_t)) return ImGuiDataType_U16;
		if (typeid(T) == typeid(int32_t)) return ImGuiDataType_S32;
		if (typeid(T) == typeid(uint32_t)) return ImGuiDataType_U32;
		if (typeid(T) == typeid(int64_t)) return ImGuiDataType_S64;
		if (typeid(T) == typeid(uint64_t)) return ImGuiDataType_U64;
		if (typeid(T) == typeid(float)) return ImGuiDataType_Float;
		if (typeid(T) == typeid(double)) return ImGuiDataType_Double;
		return ImGuiDataType_COUNT;
	}
}

// namespace std {
// 	inline std::ostream &operator<<(std::ostream &os, const Math::Vector2<float> &vec) {
// 		os << "(" << vec.x << ", " << vec.y << ")";
// 		return os;
// 	}
//
// 	inline std::ostream &operator<<(std::ostream &os, const Math::Rect &rect) {
// 		os << "(" << rect.min << ", " << rect.max << ")";
// 		return os;
// 	}
// }