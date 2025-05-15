//
// Created by radue on 3/2/2025.
//

#pragma once

#include <type_traits>
#include "element.h"
#include "magic_enum/magic_enum.hpp"

namespace Coral::Reef {
	template <typename T> requires std::is_enum_v<T>
	class DropDown final : public Element {
	public:
		DropDown(std::string name, T* value, UnorderedSet<T> excluded, const Style& style = Style())
			: Element(style), m_name(std::move(name)), m_value(value), m_excluded(std::move(excluded)) {}
		~DropDown() override = default;

		bool Render() override {
			const bool shouldReset = Element::Render();

			ImGui::SetNextItemWidth(m_currentSize.width - m_padding.left - m_padding.right);
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

			return shouldReset;
		}

	private:
		String m_name;
		T* m_value;
		UnorderedSet<T> m_excluded {};
    };
}
