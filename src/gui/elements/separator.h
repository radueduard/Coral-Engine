//
// Created by radue on 2/18/2025.
//

#pragma once

#include "element.h"
#include "imgui.h"

namespace Coral::Reef {
	class Separator final : public Element {
	public:
		explicit Separator(
			const f32 size = 1.f,
			const Style& style = Style {
				.padding = { 0.f, 0.f, 1.f, 1.f },
			}) : Element(style), m_size(size)
		{
			if (m_style.direction == Axis::Horizontal) {
				m_baseSize = { Grow, m_size + m_style.padding.top + m_style.padding.bottom };
			} else {
				m_baseSize = { m_size + m_style.padding.left + m_style.padding.right, Grow };
			}
		}
		~Separator() override = default;

		void Subrender() override {
			ImGui::SetCursorScreenPos({ m_actualRenderedPosition.x + m_style.padding.left, m_actualRenderedPosition.y + m_style.padding.top });

			ImGui::GetWindowDrawList()->AddRectFilled(
				ImVec2(m_actualRenderedPosition.x + m_style.padding.left, m_actualRenderedPosition.y + m_style.padding.top),
				ImVec2(
					m_actualRenderedPosition.x + m_currentSize.width - m_style.padding.right,
					m_actualRenderedPosition.y + m_currentSize.height - m_style.padding.bottom),
				ImGui::ColorConvertFloat4ToU32(ImVec4(.8f, .8f, .8f, 1.f ))
			);
		}

	private:
		f32 m_size = 1.f;
    };
}
