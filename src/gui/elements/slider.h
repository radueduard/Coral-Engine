//
// Created by radue on 2/20/2025.
//

#pragma once

#include <string>
#include <utility>

#include "element.h"

namespace Coral::Reef {
	template <typename T, int N> requires std::is_arithmetic_v<T> && (N >= 1) && (N <= 4)
	class Slider final : public Element {
	public:
		Slider(std::string name, T *value, T min, T max, T step = 0, const Style& style = Style())
			: Element(style), m_name(std::move(name)), m_value(value), m_min(min), m_max(max), m_step(step) {}
		~Slider() override = default;

		void Subrender() override {
			ResetState();

			ImGui::BeginChild(m_name.c_str(), ImVec2(m_currentSize), false, ImGuiWindowFlags_NoDecoration);
			ImGui::PushItemWidth(m_currentSize.width - m_style.padding.left - m_style.padding.right);
			ImGui::SliderScalarN("", GetImGuiDataType<T>(), m_value, N, &m_min, &m_max);
			ImGui::PopItemWidth();
			ImGui::EndChild();
		}

	private:
		std::string m_name;
		T *m_value;
		T m_min;
		T m_max;
		T m_step;
	};
}