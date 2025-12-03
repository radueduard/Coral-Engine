//
// Created by radue on 3/2/2025.
//

#pragma once

#include <type_traits>
#include "element.h"
#include "magic_enum/magic_enum.hpp"

namespace Coral::Reef {
	static Style DropDownDefaultStyle() {
		return {
			.cornerRadius = 5.f,
			.backgroundColor = Colors::grey[800],
		};
	};

	static Text::Style DropDownDefaultTextStyle() {
		return {
			.color = Colors::white,
			.fontSize = 15.f,
			.fontStyle = FontType::Regular,
		};
	};

	template <typename T> requires std::is_enum_v<T>
	class DropDown final : public Element {
	public:
		DropDown(
			std::string name,
			T* value, UnorderedSet<T> excluded,
			const Style& style = DropDownDefaultStyle(),
			const Text::Style& textStyle = DropDownDefaultTextStyle()
		) : Element(style), m_name(std::move(name)), m_value(value), m_excluded(std::move(excluded)), m_textStyle(textStyle) {}
		~DropDown() override = default;

		void Subrender() override {
			const f32 fontSize = std::min(
				m_currentSize.width - m_style.padding.top - m_style.padding.bottom,
				m_textStyle.fontSize);

			const Math::Vector2f padding = {
				(m_currentSize.height - fontSize),
				(m_currentSize.height - fontSize) / 2.f
			};

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(padding));
			ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, m_style.cornerRadius);
			ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(m_textStyle.color));

			ImGui::SetWindowFontScale(m_textStyle.fontSize / ImGui::GetFontSize());

			// ImGui::SetCursorScreenPos(ImVec2 { m_actualRenderedPosition - Math::Vector2f { m_style.padding } / 2.f });
			ImGui::SetNextItemWidth(m_currentSize.width);
			if (ImGui::BeginCombo(("##" + m_name).c_str(), magic_enum::enum_name(*m_value).data(), ImGuiComboFlags_HeightLarge)) {
				for (auto [value, name] : magic_enum::enum_entries<T>()) {
					if (m_excluded.contains(value) || name.empty()) {
						continue;
					}
					const bool isSelected = *m_value == value;
					if (ImGui::Selectable(name.data(), isSelected)) {
						*m_value = static_cast<T>(value);
					}
					if (isSelected) {
						ImGui::SetItemDefaultFocus();
					}
				}
				ImGui::EndCombo();
			}
			ImGui::SetWindowFontScale(1.f);

			ImGui::PopStyleVar(2);
			ImGui::PopStyleColor(1);
		}

	private:
		String m_name;
		T* m_value;
		UnorderedSet<T> m_excluded {};
		Text::Style m_textStyle;
    };
}
