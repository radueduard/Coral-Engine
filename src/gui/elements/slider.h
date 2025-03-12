//
// Created by radue on 2/20/2025.
//

#pragma once

#include <string>
#include <utility>

#include "element.h"

namespace GUI {
	template <typename T, int N, typename = std::enable_if_t<std::is_arithmetic_v<T> && N >= 1 && N <= 4>>
	class Slider final : public Element {
	public:
		Slider(std::string name, T **value, T min, T max, T step = 0)
			: m_name(std::move(name)), m_value(value), m_min(min), m_max(max), m_step(step) {}
		~Slider() override = default;

		void Render() override {
			m_outerBounds = m_parent->AllocatedArea(this);
			m_innerBounds.min = m_outerBounds.min;
			ImGui::SetCursorScreenPos(m_innerBounds.min);

			ImGui::BeginChild(m_name.c_str(), {0, 0}, false, ImGuiWindowFlags_NoDecoration);
			ImGui::PushItemWidth(250);
			ImGui::SliderScalarN(m_name.c_str(), Math::ImGuiDataType<T>, *m_value, N, &m_min, &m_max);
			ImGui::PopItemWidth();
			m_requiredArea = {250, 2 * ImGui::GetStyle().FramePadding.y + ImGui::GetFontSize()};
			m_innerBounds.max = m_innerBounds.min + m_requiredArea;
			ImGui::EndChild();
		}

	private:
		std::string m_name;
		T **m_value;
		T m_min;
		T m_max;
		T m_step;
	};
}