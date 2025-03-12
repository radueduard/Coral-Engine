//
// Created by radue on 2/20/2025.
//

#pragma once

#include <functional>
#include <memory>

#include "element.h"

namespace GUI {
	class Conditional final : public Element {
	public:
		Conditional(std::function<bool()> condition, Element* trueElement, Element* falseElement)
			: m_condition(std::move(condition)), m_trueElement(trueElement), m_falseElement(falseElement) {
			m_trueElement->AttachTo(this);
			m_falseElement->AttachTo(this);
		}
		~Conditional() override = default;

		void Render() override {
			m_outerBounds = m_parent->AllocatedArea(this);
			m_innerBounds = m_outerBounds;

			if (m_condition()) {
				m_trueElement->Render();
			} else {
				m_falseElement->Render();
			}
		}

		Math::Rect AllocatedArea(Element *element) const override {
			if (element == m_trueElement.get() || element == m_falseElement.get()) {
				return m_innerBounds;
			}
			return Math::Rect::Zero();
		}

	private:
		std::function<bool()> m_condition;
		std::unique_ptr<Element> m_trueElement;
		std::unique_ptr<Element> m_falseElement;
	};
}
