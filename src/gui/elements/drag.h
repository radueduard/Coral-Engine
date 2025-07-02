//
// Created by radue on 2/21/2025.
//

#pragma once

#include <string>
#include <utility>

#include "element.h"

namespace Coral::Reef {
	template <typename T, int N, typename = std::enable_if_t<std::is_arithmetic_v<T> && N >= 1 && N <= 4>>
	class Drag final : public Element {
	public:
		Drag(std::string name, T *value, const float speed, T min, T max, bool* changed, std::optional<std::array<ImGui::ImLabel, N>> labels = std::nullopt, const Style& style = Style())
			: Element(style), m_name(std::move(name)), m_value(value), m_speed(speed), m_min(min), m_max(max), m_changed(changed), m_labels(labels) {}
		~Drag() override = default;

		bool Render() override {
			if (Element::Render()) {
				ResetState();
			}

			ImGui::BeginChild(m_name.c_str(), {m_currentSize.width, m_currentSize.height }, false, ImGuiWindowFlags_NoDecoration);
			ImGui::PushItemWidth(m_currentSize.width - m_padding.left - m_padding.right);
			const bool changed = ImGui::DragScalarN("", GetImGuiDataType<T>(), m_value, N, m_speed, &m_min, &m_max, 0, 0, m_labels.has_value() ? m_labels->data() : nullptr);
			ImGui::PopItemWidth();
			ImGui::EndChild();
			*m_changed |= changed;
			return changed;
		}

	private:
		std::string m_name;
		T *m_value;
		f32 m_speed;
		T m_min;
		T m_max;
		bool *m_changed = nullptr;
		std::optional<std::array<ImGui::ImLabel, N>> m_labels;
	};
}