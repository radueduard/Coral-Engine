//
// Created by radue on 5/11/2025.
//


#pragma once
#include "element.h"

namespace Coral::Reef {
	class Checkbox final : public Element {
	public:
		explicit Checkbox(const bool initialValue, std::function<void(bool)> function = [](bool){}, const Style& style = Style())
			: Element(style), m_value(initialValue), m_function(std::move(function)) {}
		~Checkbox() override = default;

		bool Render() override {
			const bool shouldReset = Element::Render();

			ImGui::SetNextItemWidth(m_currentSize.width - m_padding.left - m_padding.right);
			if (ImGui::Checkbox("##", &m_value)) {
				m_function(m_value);
			}

			return shouldReset;
		}
	private:
		bool m_value;
		std::function<void(bool)> m_function;
	};
}
