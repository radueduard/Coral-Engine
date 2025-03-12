//
// Created by radue on 2/21/2025.
//

#pragma once

#include <string>
#include <utility>

#include "element.h"
#include "gui/manager.h"

namespace GUI {
	template <typename T, int N, typename = std::enable_if_t<std::is_arithmetic_v<T> && N >= 1 && N <= 4>>
	class Drag final : public Element {
	public:
		Drag(std::string name, T *value, const float speed, T min, T max, std::optional<std::array<ImGui::ImLabel, N>> labels = std::nullopt)
			: m_name(std::move(name)), m_value(value), m_speed(speed), m_min(min), m_max(max), m_labels(labels) {}
		~Drag() override = default;

		void Render() override {
			m_requiredArea = {250, 2 * ImGui::GetStyle().FramePadding.y + ImGui::GetFontSize()};
			m_outerBounds = m_parent->AllocatedArea(this);
			m_innerBounds.min = m_outerBounds.min;

			ImGui::SetCursorScreenPos(m_innerBounds.min);

			ImGui::BeginChild(m_name.c_str(), m_requiredArea, false, ImGuiWindowFlags_NoDecoration);
			ImGui::PushItemWidth(250);
			ImGui::DragScalarN(m_name.c_str(), Math::ImGuiDataType<T>(), m_value, N, m_speed, &m_min, &m_max, 0, 0, m_labels.has_value() ? m_labels->data() : nullptr);
			ImGui::PopItemWidth();
			m_innerBounds.max = m_innerBounds.min + m_requiredArea;
			ImGui::EndChild();
		}

	private:
		std::string m_name;
		T *m_value;
		float m_speed;
		T m_min;
		T m_max;
		std::optional<std::array<ImGui::ImLabel, N>> m_labels;
	};
}