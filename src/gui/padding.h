//
// Created by radue on 17/10/2025.
//

#pragma once

#include "utils/types.h"

namespace Coral::Reef {
	struct Padding {
		f32 left = 0.0f;
		f32 right = 0.0f;
		f32 top = 0.0f;
		f32 bottom = 0.0f;

		Padding() = default;
		explicit Padding(const f32 all)
			: left(all), right(all), top(all), bottom(all) {}
		Padding(const f32 horizontal, const f32 vertical)
			: left(horizontal), right(horizontal), top(vertical), bottom(vertical) {}
		Padding(const f32 left, const f32 right, const f32 top, const f32 bottom)
			: left(left), right(right), top(top), bottom(bottom) {}

		f32 Horizontal() const { return left + right; }
		f32 Vertical() const { return top + bottom; }
	};
}
