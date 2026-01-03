//
// Created by Eduard Andrei Radu on 22.12.2025.
//

#include "random.h"

#include "math/constants.h"

namespace Coral::Utils {
	template<>
	Math::Vector2f Random::Direction<2>() {
		const f32 angle = UniformRealValue(0.0f, 2.0f * Math::Pi<f32>());
		return { std::cos(angle), std::sin(angle) };
	}

	template<>
	Math::Vector3f Random::Direction<3>() {
		// Using Marsaglia (1972) method
		f32 x1, x2, s;
		do {
			x1 = UniformRealValue(-1.0f, 1.0f);
			x2 = UniformRealValue(-1.0f, 1.0f);
			s = x1 * x1 + x2 * x2;
		} while (s >= 1.0f || s == 0.0f);
		f32 z = 1.0f - 2.0f * s;
		const f32 factor = 2.0f * std::sqrt(1.0f - s);
		f32 x = x1 * factor;
		f32 y = x2 * factor;
		return { x, y, z };
	}
}