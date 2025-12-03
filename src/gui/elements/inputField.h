//
// Created by radue on 2/21/2025.
//


#pragma once
#include <string>

#include "element.h"
#include "imgui_internal.h"
#include "text.h"

namespace Coral::Reef {
	class InputField final : public Element {
	public:
		InputField(std::string name, std::string* value, const Style& style = Style(), const Text::Style& textStyle = {})
			: Element(style), m_name(std::move(name)), m_value(value), m_textStyle(textStyle) {}
		~InputField() override = default;

		void Subrender() override {
			ImGui::PushID(this);
			ImGui::SetWindowFontScale(m_textStyle.fontSize / ImGui::GetFontSize());
			ImGui::InputTextEx(
				("##" + m_name).c_str(),
				"object",
				m_value->data(),
				static_cast<i32>(m_value->capacity()),
				{
					m_currentSize.width,
					m_currentSize.height,
				},
				ImGuiInputTextFlags_None
			);
			ImGui::SetWindowFontScale(1.f);
			ImGui::PopID();
		}

	private:
		std::string m_name;
		std::string* m_value;
		Text::Style m_textStyle;
	};
}
