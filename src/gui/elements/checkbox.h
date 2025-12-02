//
// Created by radue on 5/11/2025.
//


#pragma once
#include "element.h"

namespace Coral::Reef {
	class Checkbox final : public Element {
	public:
		explicit Checkbox(String name, const bool initialValue, std::function<void(bool)> function = [](bool){}, const Style& style = Style())
			: Element(style), m_name(std::move(name)), m_value(initialValue), m_function(std::move(function)) {}
		~Checkbox() override = default;

		void Subrender() override {
			ImGui::SetNextItemWidth(m_currentSize.width);
			if (ImGui::Checkbox(("##" + m_name).c_str(), &m_value)) {
				m_function(m_value);
			}
		}
	private:
		String m_name;
		bool m_value;
		std::function<void(bool)> m_function;
	};
}
