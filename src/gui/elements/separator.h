//
// Created by radue on 2/18/2025.
//

#pragma once

#include <imgui_internal.h>

#include "element.h"
#include "imgui.h"

namespace Coral::Reef {
	class Separator final : public Element {
	public:
		explicit Separator(
			const f32 size = 1.f,
			const Style& style = Style {
				.padding = { 0.f, 0.f, 1.f, 1.f },
				.backgroundColor = { .8f, .8f, .8f, 1.f },
			}) : Element(style), m_size(size)
		{
			if (m_style.direction == Axis::Horizontal) {
				m_baseSize = { Grow, m_size + m_style.padding.top + m_style.padding.bottom };
			} else {
				m_baseSize = { m_size + m_style.padding.left + m_style.padding.right, Grow };
			}
		}
		~Separator() override = default;
	private:
		f32 m_size = 1.f;
    };
}
