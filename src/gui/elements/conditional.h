//
// Created by radue on 5/10/2025.
//

#pragma once
#include <functional>

#include "element.h"

namespace Coral::Reef {
	class Conditional final : public Element {
	public:
		explicit Conditional(const std::function<bool()>& condition, Element* trueElement, Element* falseElement) :
			m_trueElement(trueElement), m_falseElement(falseElement), m_condition(condition) {
			m_children.emplace_back(std::move(m_trueElement));
		}

		bool Render() override {
			m_state = m_condition();
			if (m_state != m_lastState) {
				m_lastState = m_state;
				ResetState([this] {
					if (m_state) {
						m_falseElement = std::move(m_children.back());
						m_children.clear();
						m_children.emplace_back(std::move(m_trueElement));
					} else {
						m_trueElement = std::move(m_children.back());
						m_children.clear();
						m_children.emplace_back(std::move(m_falseElement));
					}
				});
			}

			return Element::Render();
		}
	private:
		bool m_state = true;
		bool m_lastState = true;
		std::unique_ptr<Element> m_trueElement;
		std::unique_ptr<Element> m_falseElement;
		std::function<bool()> m_condition;
	};
}
