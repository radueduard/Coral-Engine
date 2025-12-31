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
		return Text::Style()
			.withColor(Colors::white)
			.withFontSize(15.f)
			.withFontStyle(FontType::Regular);
	}

	template <typename T = f32, int N = 1> requires std::is_arithmetic_v<T> && (N >= 1) && (N <= 4)
	class Drag final : public Element {
	public:
		Drag(
			std::string name,
			std::array<T*, N> value,
			const f32 speed,
			std::array<T, N> min,
			std::array<T, N> max,
			bool* changed,
			std::optional<std::array<ImGui::ImLabel, N>> labels = std::nullopt,
			const Style& style = DragDefaultStyle(),
			const Text::Style& textStyle = DragDefaultTextStyle()
		) : Element(style), m_name(std::move(name)), m_value(value), m_speed(speed), m_min(min), m_max(max), m_changed(changed), m_labels(labels), m_textStyle(textStyle) {
			localValue = new T[N];
			for (int i = 0; i < N; ++i) {
				localValue[i] = *m_value[i];
			}
		}
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

			ImGui::PushItemWidth(m_currentSize.width);
			const bool changed = ImGui::DragScalarN(
				"",
				GetImGuiDataType<T>(),
				localValue,
				N,
				m_speed,
				m_min.data(),
				m_max.data(),
				0,
				0,
				m_labels.has_value() ? m_labels->data() : nullptr
			);
			if (changed) {
				for (int i = 0; i < N; ++i) {
					*m_value[i] = localValue[i];
				}
			}

			ImGui::PopItemWidth();

			ImGui::SetWindowFontScale(1.f);

			ImGui::PopStyleVar(2);
			ImGui::PopStyleColor(1);

			if (m_changed != nullptr)
				*m_changed |= changed;
		}

	private:
		T* localValue;

		std::string m_name;
		std::array<T*, N> m_value;
		f32 m_speed;
		std::array<T, N> m_min;
		std::array<T, N> m_max;
		bool *m_changed = nullptr;
		std::optional<std::array<ImGui::ImLabel, N>> m_labels;
		Text::Style m_textStyle;
	};
}