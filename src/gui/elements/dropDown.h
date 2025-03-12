//
// Created by radue on 3/2/2025.
//


#pragma once
#include <type_traits>
#include <utility>

#include "element.h"

namespace GUI {
	template <typename T, typename = std::enable_if_t<std::is_enum_v<T>>, typename = std::enable_if_t<std::is_same_v<std::string, decltype(std::to_string(std::declval<T>()))>>>
	class DropDown final : public Element {
	public:
		DropDown(std::string name, T* value) : m_name(std::move(name)), m_value(value) {}
		~DropDown() override = default;

		void Render() override {
			m_requiredArea = { 250 , 2 * ImGui::GetStyle().FramePadding.y + ImGui::GetFontSize() + 2 * ImGui::GetStyle().FrameBorderSize };
			m_outerBounds = m_parent->AllocatedArea(this);
			m_innerBounds = m_outerBounds;

			ImGui::SetCursorScreenPos(m_innerBounds.min);

			ImGui::SetNextItemWidth(m_requiredArea.x);
			if (ImGui::BeginCombo(("##" + m_name).c_str(), std::to_string(*m_value).c_str())) {
				for (int i = 0; i < static_cast<int>(T::Count); i++) {
					const bool isSelected = *m_value == static_cast<T>(i);
					if (ImGui::Selectable(std::to_string(static_cast<T>(i)).c_str(), isSelected)) {
						*m_value = static_cast<T>(i);
					}
					if (isSelected) {
						ImGui::SetItemDefaultFocus();
					}
				}
				ImGui::EndCombo();
			}
		}

	private:
		std::string m_name;
		T* m_value;
    };
}
