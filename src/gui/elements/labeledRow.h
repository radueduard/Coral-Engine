//
// Created by radue on 3/2/2025.
//

#pragma once

#include <memory>

#include "element.h"
#include "text.h"

namespace Coral::Reef {
	class LabeledRow final : public Element {
	public:
		LabeledRow(Text* label, Element* element, const Style& style = Style())
			: Element(style) {
			m_axis = Axis::Horizontal;
			m_children.emplace_back(label);
			if (element->m_baseSize.width != Grow) {
				m_children.emplace_back(new Element());
			}
			m_children.emplace_back(element);
		}
		~LabeledRow() override = default;
    };
}
