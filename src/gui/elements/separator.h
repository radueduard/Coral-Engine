//
// Created by radue on 2/18/2025.
//

#pragma once

#include <imgui_internal.h>

#include "element.h"
#include "imgui.h"

namespace GUI {
	class Separator final : public Element {
	public:
		enum Direction {
            Horizontal,
            Vertical
        };

		explicit Separator(const Direction direction = Horizontal, const float size = 1.f) : m_size(size), m_direction(direction) {}
		~Separator() override = default;

		void Render() override {
			m_outerBounds = m_parent->AllocatedArea(this);
			m_innerBounds = m_outerBounds;
			m_requiredArea = { 0, m_size };

			ImGui::SetCursorScreenPos(m_innerBounds.min);

			ImGui::SeparatorEx(
				ImGuiSeparatorFlags_SpanAllColumns | (m_direction == Horizontal ? ImGuiSeparatorFlags_Horizontal : ImGuiSeparatorFlags_Vertical),
				m_size
			);
		}
	private:
		float m_size = 1.f;
		Direction m_direction = Horizontal;
    };
}
