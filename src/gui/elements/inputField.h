//
// Created by radue on 2/21/2025.
//


#pragma once
#include <string>

#include "element.h"

namespace GUI {
	class InputField final : public Element {
	public:
		InputField(std::string name, std::string* value) : m_name(std::move(name)), m_value(value) {}
		~InputField() override = default;

		void Render() override {
			m_outerBounds = m_parent->AllocatedArea(this);
			m_innerBounds.min = m_outerBounds.min;
			ImGui::SetCursorScreenPos(m_innerBounds.min);

			ImGui::PushItemWidth(250);
			ImGui::InputText(("##" + m_name).c_str(), m_value->data(), m_value->capacity());
			ImGui::PopItemWidth();
			m_requiredArea = { 250, 2 * ImGui::GetStyle().FramePadding.y + ImGui::GetFontSize() };
			m_innerBounds.max = m_innerBounds.min + m_requiredArea;
		}

	private:
		std::string m_name;
		std::string* m_value;
	};
}
