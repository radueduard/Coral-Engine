//
// Created by radue on 17/10/2025.
//

#pragma once

#include "math/vector.h"

namespace Coral
{
	struct Color
	{
		f32 r, g, b, a;

		/**
		 * Create an 8-bit color from a hexadecimal value.
		 *
		 * @param hex the color in 0xAARRGGBB format
		 */
		constexpr explicit Color(const u32 hex)
		{
			a = static_cast<f32>((hex >> 24) & 0xFF) / 255.0f;
			r = static_cast<f32>((hex >> 16) & 0xFF) / 255.0f;
			g = static_cast<f32>((hex >> 8) & 0xFF) / 255.0f;
			b = static_cast<f32>((hex >> 0) & 0xFF) / 255.0f;
		};

		constexpr Color(const f32 red, const f32 green, const f32 blue, const f32 alpha = 1.0f)
			: r(red), g(green), b(blue), a(alpha) {}

		constexpr explicit Color(const Math::Vector4f& vec) : r(vec.r), g(vec.g), b(vec.b), a(vec.a) {}
		constexpr explicit Color(const Math::Vector3f& vec) : r(vec.r), g(vec.g), b(vec.b), a(1.0f) {}

		constexpr explicit operator Math::Vector4f() const { return Math::Vector4f{ r, g, b, a }; }
		constexpr explicit operator ImVec4() const { return { r, g, b, a }; }
		constexpr explicit operator Math::Vector3f() const { return Math::Vector3f{ r, g, b }; }

		Color transparency(const f32 alpha) const
		{
			return Color(r, g, b, a * alpha);
		}
	};

	class Colors
	{
		constexpr static u32 clampIndex(u32 index)
		{
			index = index % 1000;
			if (index < 100) index = 0;
			else if (index < 200) index = 1;
			else if (index < 300) index = 2;
			else if (index < 400) index = 3;
			else if (index < 500) index = 4;
			else if (index < 600) index = 5;
			else if (index < 700) index = 6;
			else if (index < 800) index = 7;
			else if (index < 900) index = 8;
			else index = 9;
			return index;
		}

		explicit Colors(const std::array<Color, 10>& colors, const u32 def = 500) : m_default(def), m_colors(colors) {}

		u32 m_default = 500;
		std::array<Color, 10> m_colors;
	public:
		constexpr Color operator[](u32 index) const
		{
			index = clampIndex(index);
			return m_colors[index];
		}

		constexpr operator Color() const { return m_colors[clampIndex(m_default)]; }
		constexpr operator ImVec4() const { return ImVec4 { m_colors[clampIndex(m_default)].r, m_colors[clampIndex(m_default)].g, m_colors[clampIndex(m_default)].b, m_colors[clampIndex(m_default)].a }; }

		static const Colors pink;
		static const Colors red;
		static const Colors deepOrange;
		static const Colors orange;
		static const Colors amber;
		static const Colors yellow;
		static const Colors lime;
		static const Colors lightGreen;
		static const Colors green;
		static const Colors teal;
		static const Colors cyan;
		static const Colors lightBlue;
		static const Colors blue;
		static const Colors indigo;
		static const Colors purple;
		static const Colors deepPurple;
		static const Colors blueGrey;
		static const Colors brown;
		static const Colors grey;

		static const Colors black;
		static const Colors white;

		static const Colors transparent;
	};
}
