//
// Created by Eduard Andrei Radu on 22.12.2025.
//

#include "random.h"

namespace Coral::Utils {
	template<>
	Math::Vector2f Random::Direction<2>() {
		return NormalVector<2, f32>(Math::Vector2f(0.0f, 0.0f), Math::Vector2f(1.0f, 1.0f)).Normalized();
	}

	template<>
	Math::Vector3f Random::Direction<3>() {
		return NormalVector<3, f32>(Math::Vector3f(0.0f, 0.0f, 0.0f), Math::Vector3f(1.0f, 1.0f, 1.0f)).Normalized();
	}
}