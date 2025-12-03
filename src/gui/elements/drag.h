//
// Created by radue on 2/21/2025.
//

#pragma once

#include <string>
#include <utility>

#include "element.h"

namespace Coral::Reef {
	static Style DragDefaultStyle() {
		return {
			.cornerRadius = 5.f,
			.backgroundColor = Colors::transparent
		};
	};

	static Text::Style DragDefaultTextStyle() {
		return {
			.color = Colors::white,
			.fontSize = 15.f,
			.fontStyle = FontType::Regular,
		};
	};

	template <typename T = f32, int N = 1> requires std::is_arithmetic_v<T> && (N >= 1) && (N <= 4)
	class Drag final : public Element {
	public:
		Drag(
			std::string name,
			T *value,
			const float speed,
			T min,
			T max,
			bool* changed,
			std::optional<std::array<ImGui::ImLabel, N>> labels = std::nullopt,
			const Style& style = DragDefaultStyle(),
			const Text::Style& textStyle = DragDefaultTextStyle()
		) : Element(style), m_name(std::move(name)), m_value(value), m_speed(speed), m_min(min), m_max(max), m_changed(changed), m_labels(labels), m_textStyle(textStyle) {}
		~Drag() override = default;

		void Subrender() override {
			ResetState();

			const f32 fontSize = std::min(
				m_currentSize.height - m_style.padding.top - m_style.padding.bottom,
				m_textStyle.fontSize);

			const Math::Vector2f padding = {
				(m_currentSize.height - fontSize),
				(m_currentSize.height - fontSize) / 2.f
			};

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(padding.x, padding.y));
			ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, m_style.cornerRadius);
			ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(m_textStyle.color));

			ImGui::SetWindowFontScale(m_textStyle.fontSize / ImGui::GetFontSize());

			// ImGui::SetCursorScreenPos(ImVec2 { m_actualRenderedPosition - Math::Vector2f { m_style.padding } / 2.f });
			ImGui::PushItemWidth(m_currentSize.width);
			const bool changed = ImGui::DragScalarN("", GetImGuiDataType<T>(), m_value, N, m_speed, &m_min, &m_max, 0, 0, m_labels.has_value() ? m_labels->data() : nullptr);
			ImGui::PopItemWidth();

			ImGui::SetWindowFontScale(1.f);

			ImGui::PopStyleVar(2);
			ImGui::PopStyleColor(1);

			if (m_changed != nullptr)
				*m_changed |= changed;
		}

	private:
		std::string m_name;
		T *m_value;
		f32 m_speed;
		T m_min;
		T m_max;
		bool *m_changed = nullptr;
		std::optional<std::array<ImGui::ImLabel, N>> m_labels;
		Text::Style m_textStyle;
	};
}