//
// Created by radue on 2/21/2025.
//


#pragma once
#include <string>

#include "element.h"
#include "imgui_internal.h"

namespace Coral::Reef {
	class InputField final : public Element {
	public:
		InputField(std::string name, std::string* value, const Style& style = Style())
			: Element(style), m_name(std::move(name)), m_value(value) {}
		~InputField() override = default;

		bool Render() override {
			const bool shouldReset = Element::Render();

			ImGui::PushID(this);
			// ImGui::SetCursorScreenPos(m_position + Math::Vector2 { m_padding.left, m_padding.top });
			ImGui::InputTextEx(
				("##" + m_name).c_str(),
				"object",
				m_value->data(),
				static_cast<i32>(m_value->capacity()),
				{
					m_currentSize.width - m_padding.left - m_padding.right,
					m_currentSize.height - m_padding.top - m_padding.bottom,
				},
				ImGuiInputTextFlags_None
			);
			ImGui::PopID();

			return shouldReset;
		}

	private:
		std::string m_name;
		std::string* m_value;
	};
}
